module.exports = {
  /**
   * It defines structure for display fields in web forms
   * @readonly
   * @enum {Form}
   */
  Form: {
    /**
     * A type defines how group values
     */
    Type: {
      OBJECT: 'object', // It represents a fieldset
      ARRAY: 'array',   // It represents an array, a tab set
      STRING: 'text'    // It represents either select (combobox) or a simple text
    },

    /**
     * A field defines how to display a field. It design a input
     */
    Field: {
      TEXT: "string",
      NUMBER: "number",
      CHECKBOX: "boolean",
      COMBOBOX: "string",
      TEXTAREA: "textarea",
      PASSWORD: "password"
    }
  },

  /**
   * It is used for handling structure for custom request syntax
   * @readonly
   * @enum {FormField}
  */
  Uri: {
    HOST: 'hostname',
    PORT: 'port',
    SCHEME: 'protocol',
    PATHNAME: 'pathname',
    AUTH: 'auth',
    USER: 'user',
    PASSWORD: 'password'
  },

  DataSeriesType: {
    DCP: 'DCP',
    OCCURRENCE: 'OCCURRENCE',
    GRID: 'GRID',
    ANALYSIS_MONITORED_OBJECT: 'ANALYSIS_MONITORED_OBJECT',
    STATIC_DATA: 'STATIC_DATA',
    POSTGIS: 'POSTGIS'
  },

  AnalysisType: {
    DCP: 1,
    GRID: 2,
    MONITORED: 3
  },

  DataSeriesFormat: {
    CSV: 'CSV',
    POSTGIS: 'POSTGIS',
    OGR: 'OGR',
    GEOTIFF: 'GEOTIFF'
  },

  DataSeriesSemantics: {
    NAME: "name",
    FORMAT: "data_format_name",
    TYPE: "data_series_type_name"
  },

  DataProviderIntent: {
    COLLECT: 'COLLECT',
    PROCESSING: 'PROCESSING'
  },

  DataProviderIntentId: {
    COLLECT: 1,
    PROCESSING: 2
  },

  /**
   * It is used for representing output formats
   * @readonly
   * @enum {Format}
   */
  Format: {
    GEOJSON: 'GeoJSON',
    WKT: 'wkt'
  },

  TokenCode: {
    SAVE: 1,
    UPDATE: 2,
    DELETE: 3
  },

  InfluenceTypes: {
    TOUCHES: {
      Name: "Touches",
      Value: 1
    },
    CENTER: {
      Name: "Center",
      Value: 2
    },
    REGION: {
      Name: "Region",
      Value: 3
    }
  },

  ScriptLanguage: {
    PYTHON: 1,
    LUA: 2
  }
};
