module.exports = function(sequelize, DataTypes) {
  var DataSetMonitored = sequelize.define("DataSetMonitored",
    {
      id: {
        type: DataTypes.INTEGER,
        allowNull: false,
        primaryKey: true,
        autoIncrement: true
      },
      time_column: DataTypes.STRING,
      geometry_column: DataTypes.STRING,
      id_column: DataTypes.STRING,
      srid: DataTypes.INTEGER
    },
    {
      underscored: true,
      underscoredAll: true,
      timestamps: false,

      classMethods: {
        associate: function(models) {
          DataSetMonitored.belongsTo(models.DataSet, {
            onDelete: "CASCADE",
            foreignKey: {
              allowNull: false
            }
          });
          //
          //DataSetMonitored.belongsTo("spatial_ref_sys",
          //  {foreignKey: {name: 'fk_spatial_ref_sys_srid', targetKey: 'name'}});
        }
      }
    }
  );

  return DataSetMonitored;
};