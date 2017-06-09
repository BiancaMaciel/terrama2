module.exports = function(sequelize, DataTypes) {
  var Alert = sequelize.define("Alert",
    {
      id: {
        type: DataTypes.INTEGER,
        allowNull: false,
        primaryKey: true,
        autoIncrement: true
      },
      active: DataTypes.BOOLEAN,
      name: DataTypes.STRING,
      description: DataTypes.TEXT,
      risk_attribute: DataTypes.STRING,
      schedule_type: {
        type: DataTypes.INTEGER,
        allowNull: true
      }
    },
    {
      underscored: true,
      underscoredAll: true,
      timestamps: false,

      classMethods: {
        associate: function(models) {
          
          Alert.belongsTo(models.Project, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'project_id',
              allowNull: false
            }
          });

          Alert.belongsTo(models.DataSeries, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'data_series_id',
              allowNull: false
            }
          });

          Alert.belongsTo(models.ServiceInstance, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'service_instance_id',
              allowNull: false
            }
          });

          Alert.belongsTo(models.Schedule, {
            onDelete: "CASCADE",
            foreignKey: {
              name: "schedule_id",
              allowNull: true,
              constraints: true
            }
          });

          Alert.belongsTo(models.AutomaticSchedule, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'automatic_schedule_id',
              allowNull: true
            }
          });

          Alert.belongsTo(models.Risk, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'risk_id',
              allowNull: false
            }
          });

          Alert.hasOne(models.ReportMetadata, {
            onDelete: "CASCADE",
            foreignKey: {
              name: 'alert_id',
              allowNull: false
            }
          });

          Alert.hasMany(models.AlertAdditionalData, {
            onDelete: 'CASCADE',
            foreignKey: {
              allowNull: false
            }
          });

          Alert.hasMany(models.AlertNotification, {
            onDelete: 'CASCADE',
            foreignKey: {
              allowNull: false
            }
          });
        }
      }
    }
  );

  return Alert;
};
