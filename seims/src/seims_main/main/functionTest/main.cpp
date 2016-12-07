/*!
 * \brief Functional test
 * \author LiangJun Zhu
 * \date May 2016
 *
 * 
 */
#include <iostream>
#include "util.h"
#include "gdal_priv.h"
//#include "omp.h"
#include "mongoc.h"
//#include <string>
#include "clsRasterData.h"

#define TEST

int main(int argc, const char *argv[])
{
    GDALAllRegister();
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    bson_t *query = bson_new();
    const bson_t *doc;
    bson_error_t *err = NULL;
    char *str;
    //char* dbname = "climate_dianbu";
    const char *dbname = "model_dianbu_30m_longterm";
    //char* collname = "DataValues";
    const char *collname = "SPATIAL";
    const char *newGridFS = "OUTPUT";
    mongoc_init();
    mongoc_uri_t *uri;
    //const char *hostname = "127.0.0.1";
	const char *hostname = "192.168.6.55";
    uint16_t port = 27017;
    uri = mongoc_uri_new_for_host_port(hostname, port);
    client = mongoc_client_new_from_uri(uri);
    //mongoc_collection_t	*collection;
	mongoc_gridfs_t *gfs = mongoc_client_get_gridfs(client, dbname, collname, err);
    mongoc_gridfs_t *outgfs = mongoc_client_get_gridfs(client, dbname, newGridFS, err);

    //clsRasterData *test = new clsRasterData("E:\\data\\Dianbu\\patch_partition\\dianbu\\flow_dir.tif");

    clsRasterData *mask = new clsRasterData(gfs, "1_MASK", NULL);
	int row = 0, col =0;
	float **data;
	//Read2DArrayFromMongoDB(gfs, string("LANDUSELOOKUP"),row,col,data);
	//clsRasterData *raster1d = new clsRasterData(outgfs, "1_INLO_SUM", mask);
	//string rs1dtiffile = "G:/1_INLO_SUM.tif";
	//raster1d->outputGTiff(rs1dtiffile);
    //clsRasterData *raster1d = new clsRasterData(gfs, "1_PHU0", mask);
    //clsRasterData *raster2d = new clsRasterData(gfs,"1_DENSITY",mask);
    //string rs1dascfile = "e:/test/1_blai_asc.asc";
    //string rs2dascfile = "e:/test/1_density_asc.asc";
    //string rs1dtiffile = "G:/code_zhulj/SEIMS/model_data/TEST/model_dianbu_30m_longterm/OUTPUT/1_PHU0.tif";
    //string rs2dtiffile = "e:/test/1_density_asc.tif";
    //string rs1dMongo = "2_BLAI";
    //string rs2dMongo = "2_DENSITY";
    //raster1d->outputASCFile(rs1dascfile);
    //raster2d->outputASCFile(rs2dascfile);
    //raster1d->outputGTiff(rs1dtiffile);
    //raster2d->outputGTiff(rs2dtiffile);
    //raster1d->outputToMongoDB(rs1dMongo, gfs);
    //raster2d->outputToMongoDB(rs2dMongo, gfs);

	//string maskfile = "G:/code_zhulj/SEIMS/model_data/TEST/model_dianbu_30m_longterm/OUTPUT/1_MASK.tif";
	//mask->outputGTiff(maskfile);
    //bson_destroy(query);
    //mongoc_cursor_destroy(cursor);
    mongoc_gridfs_destroy(gfs);
    //mongoc_gridfs_destroy(newGridFS);
    //mongoc_collection_destroy(collection);
    //mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();
    system("pause");
    return 0;
}