/*
 * ids.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: wangli
 */

#include "ids.h"
#include "../Config.h"
std::string PartitionID::getPathAndName() const {

	std::ostringstream str;
//		str<<"/home/claims/data/tpc-h/4_partitions/SF-20/T"<<projection_id.table_id<<"G"<<projection_id.projection_off<<"P"<<partition_off;
//		str<<"/home/claims/data/wangli/T"<<projection_id.table_id<<"G"<<projection_id.projection_off<<"P"<<partition_off;
	str<<Config::data_dir<<"T"<<projection_id.table_id<<"G"<<projection_id.projection_off<<"P"<<partition_off;
	return str.str();
}
