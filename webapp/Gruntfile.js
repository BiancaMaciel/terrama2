module.exports = function(grunt) {
  // Project configuration
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    banner: '/*! <%= pkg.name %> <%= pkg.version %> | (C) 2007, <%= grunt.template.today("yyyy") %> National Institute For Space Research (INPE) - Brazil | https://github.com/TerraMA2/terrama2/blob/master/LICENSE */',
    requirejs: {
      TerraMA2WebApp: {
        options: {
          baseUrl: "public/javascripts",
          out: "public/dist/terrama2-webapp.min.js",
          preserveLicenseComments: false,
          // optimize: "none", // It does not minify
          paths: {
            TerraMA2WebApp: "angular"
          },
          include: [
            "../../bower_components/requirejs/require",
            "TerraMA2WebApp/application"
          ]
        }
      }
    }
  });

  grunt.loadNpmTasks('grunt-contrib-requirejs');

  grunt.registerTask('default', ['requirejs']);
};