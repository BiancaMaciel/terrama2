module.exports = function (app) {

    var controller = app.controllers.configuration.Alerts;

    app.get('/configuration/alerts', controller.index);
    app.get('/configuration/alert', controller["new"]);
}