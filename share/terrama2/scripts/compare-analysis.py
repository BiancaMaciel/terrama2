import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT

def checkTable(query):
    conn = psycopg2.connect("dbname='"+dbname+"' user='postgres' password='postgres' host = '127.0.0.1' port= '5432' ")
    conn.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
    cur = conn.cursor()
    cur.execute(query)
    rows = cur.fetchall()
    if rows:
	return 0
    else:
	return 1


typeAn = typeAnalysis
if(typeAn == "dcp_history" or  typeAn == "operator_dcp" or  typeAn == "operator_history_interval"):
    tableName = "analysis_dcp_result"
    query = "SELECT result.*, reference.* FROM "+tableName+" result INNER JOIN "+tableName+"_ref reference ON (result.id = reference.id) WHERE NOT EXISTS (SELECT * FROM "+tableName+"_ref WHERE result.execution_date = reference.execution_date and result.max = reference.max and result.count = reference.count and result.mean = reference.mean and result.median = reference.median and result.min = reference.min and result.sum = reference.sum and result.value = reference.value and result.standard_deviation = reference.standard_deviation and result.variance = reference.variance)"
    status = checkTable(query)
elif (typeAn == "occ"):
    tableName = "occurrence_analysis_result"
    #query = "SELECT result.*, reference.* FROM "+tableName+" result INNER JOIN "+tableName+"_ref reference ON (result.fid = reference.fid) WHERE NOT EXISTS (SELECT * FROM "+tableName+"_ref WHERE result.execution_date = reference.execution_date and result.count = reference.count)"
    query = "SELECT execution_date, count FROM "+tableName+"_ref WHERE NOT EXISTS (SELECT * FROM "+tableName+" WHERE "+tableName+".execution_date = "+tableName+"_ref.execution_date and "+tableName+".count = "+tableName+"_ref.count )"
    status = checkTable(query)
