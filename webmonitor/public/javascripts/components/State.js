'use strict';

define(
  ['TerraMA2WebComponents', 'components/Layers', 'components/Slider', 'components/AddLayerByUri', 'components/Utils'],
  function(TerraMA2WebComponents, Layers, Slider, AddLayerByUri, Utils) {
    var getState = function() {
      var visibleLayers = [];
      var layersOrder = [];
      var opacities = {};
      var layersDateInfo = {};
      var customLayers = [];
      var layers = Layers.getLayersByProject($("#projects").val(), true, true);

      for(var i = 0, layersLength = layers.length; i < layersLength; i++) {
        if(!layers[i].isParent) {
          if(layers[i].dateInfo && ((layers[i].dateInfo.startFilterDate !== undefined && layers[i].dateInfo.endFilterDate !== undefined) || layers[i].dateInfo.initialDateIndex !== undefined)) {
            if(layers[i].dateInfo.startFilterDate !== undefined && layers[i].dateInfo.endFilterDate !== undefined) {
              layersDateInfo[layers[i].id] = {
                startFilterDate: layers[i].dateInfo.startFilterDate,
                endFilterDate: layers[i].dateInfo.endFilterDate
              };
            } else {
              layersDateInfo[layers[i].id] = {
                initialDateIndex: layers[i].dateInfo.initialDateIndex
              };
            }
          }

          if(layers[i].visible) {
            visibleLayers.push(layers[i].id);
            layersOrder.push({
              id: layers[i].id,
              order: $("#terrama2-sortlayers > ul > #" + layers[i].htmlId).index()
            });
          }

          if(layers[i].custom) {
            var customLayer = $.extend({}, layers[i]);
            customLayer.visible = false;
            customLayers.push(customLayer);
          }

          opacities[layers[i].id] = layers[i].opacity;
        }
      }

      var state = {
        project: $("#projects").val(),
        visibleLayers: visibleLayers,
        layersOrder: layersOrder,
        opacities: opacities,
        layersDateInfo: layersDateInfo,
        extent: TerraMA2WebComponents.MapDisplay.getCurrentExtent(),
        isLegendsOpen: !$('#legend-box').hasClass('hidden'),
        isTableOpen: !($('#table-div').css('display') === 'none'),
        customLayers: customLayers
      };

      return state;
    };

    var setState = function(state) {
      $("#projects").val(state.project);

      for(var i = 0, layersLength = state.customLayers.length; i < layersLength; i++)
        AddLayerByUri.addCustomLayer(state.customLayers[i]);

      var layers = Layers.getLayersByProject($("#projects").val(), true, true);

      for(var i = 0, layersLength = layers.length; i < layersLength; i++) {
        if(!layers[i].isParent) {
          for(var j = 0, visibleLayersLength = state.visibleLayers.length; j < visibleLayersLength; j++) {
            if(layers[i].id === state.visibleLayers[j]) {
              if(!$("#" + layers[i].htmlId + " > input").prop('checked')) $("#" + layers[i].htmlId + " > input").click();
            }
          }

          if(state.opacities[layers[i].id] !== undefined) {
            Slider.changeLayerOpacity(layers[i].id, state.opacities[layers[i].id] * 100);
            Layers.setLayerOpacity(layers[i].id, state.opacities[layers[i].id]);
          }

          if(state.layersDateInfo[layers[i].id] !== undefined) {
            if(state.layersDateInfo[layers[i].id].initialDateIndex !== undefined) {
              layers[i].dateInfo.initialDateIndex = state.layersDateInfo[layers[i].id].initialDateIndex;

              Slider.doSlide(layers[i].id, layers[i].dateInfo.dates[layers[i].dateInfo.initialDateIndex]);
            } else {
              layers[i].dateInfo.startFilterDate = state.layersDateInfo[layers[i].id].startFilterDate;
              layers[i].dateInfo.endFilterDate = state.layersDateInfo[layers[i].id].endFilterDate;

              var layerTime = layers[i].dateInfo.startFilterDate + "Z/" + layers[i].dateInfo.endFilterDate + "Z";
              TerraMA2WebComponents.MapDisplay.updateLayerTime(layers[i].id, layerTime);
            }
          }
        }
      }

      var newOrder = Utils.orderByProperty(state.layersOrder, "order", "desc");
      var sortableLength = $("#terrama2-sortlayers > ul > li").length - 1;

      for(var i = 0, layersLength = newOrder.length; i < layersLength; i++) {
        var layer = Layers.getLayerById(newOrder[i].id);
        
        if(layer !== null && $("#terrama2-sortlayers > ul > #" + layer.htmlId).length > 0) {
          TerraMA2WebComponents.MapDisplay.alterLayerIndex($("#terrama2-sortlayers > ul > #" + layer.htmlId).attr('data-parentid'), (sortableLength - $("#terrama2-sortlayers > ul > #" + layer.htmlId).index()), sortableLength);
          $("#terrama2-sortlayers > ul").insertAt(0, $("#terrama2-sortlayers > ul > #" + layer.htmlId).detach());
        }
      }

      TerraMA2WebComponents.MapDisplay.zoomToExtent(state.extent);

      if(state.isLegendsOpen)
        $('#legendsButton > button').click();

      if(state.isTableOpen)
        $('#tableButton > button').click();
    };

    return {
      getState: getState,
      setState: setState
    };
  }
);