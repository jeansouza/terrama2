module.exports = function (app) {

    var controller = app.controllers.Login;

    app.get('/login', controller);
}