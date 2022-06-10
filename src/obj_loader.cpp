#include <fstream>
#include <sstream>

#include "obj_loader.h"

bool LoadObj(const std::string& _filepath, attrib_t& _attrib, index_t& _index){

	// 打开文件流
	std::ifstream fin(_filepath);
	std::string line;
	if (!fin.is_open()){
		std::cout << "file:" << _filepath << "read error" << std::endl;
		return false;
	}

	// 以一行的数据作为 string stream 解析并且读取
	while (std::getline(fin, line)){
		std::istringstream sin(line);   
		std::string linetype;
		
		// 读取时暂存
		float x, y, z;
		int v0, vt0, vn0;   
		int v1, vt1, vn1;   
		int v2, vt2, vn2;
		char slash;

		// 读取obj文件
		sin >> linetype;
		if(linetype == "v"){
			sin >> x >> y >> z;
			_attrib.vertexPosition.push_back(glm::vec3(x, y, z));
			//std::cout << linetype << " " << x << " " << y << " " << z << "\n";
		}
		if(linetype == "vt"){
			sin >> x >> y;
			_attrib.vertexTexcoord.push_back(glm::vec2(x, y));
			//std::cout << linetype << " " << x << " " << y << "\n";
		}
		if(linetype == "vn") {
			sin >> x >> y >> z;
			_attrib.vertexNormal.push_back(glm::vec3(x, y, z));
			//std::cout << linetype << " " << x << " " << y << " " << z << "\n";
		}
		if(linetype == "f") {
			sin >> v0 >> slash >> vt0 >> slash >> vn0;
			sin >> v1 >> slash >> vt1 >> slash >> vn1;
			sin >> v2 >> slash >> vt2 >> slash >> vn2;
			_index.positionIndex.push_back(glm::ivec3(v0 - 1, v1 - 1, v2 - 1));
			_index.texcoordIndex.push_back(glm::ivec3(vt0 - 1, vt1 - 1, vt2 - 1));
			_index.normalIndex.push_back(glm::ivec3(vn0 - 1, vn1 - 1, vn2 - 1));
			//std::cout << linetype;
			//std::cout << " " << v0 << "/" << vt0 << "/" << vn0 << " ";
			//std::cout << " " << v1 << "/" << vt1 << "/" << vn1 << " ";
			//std::cout << " " << v2 << "/" << vt2 << "/" << vn2 << "\n";
		}
	}

	//std::cout << "positions" << std::endl;
	//for (const auto& p : _attrib.vertexPosition) {
	//	std::cout << p.x << " " << p.y << " " << p.z << std::endl;
	//}

	//std::cout << "normals" << std::endl;
	//for (const auto& n : _attrib.vertexNormal) {
	//	std::cout << n.x << " " << n.y << " " << n.z << std::endl;
	//}

	//std::cout << "texcoords" << std::endl;
	//for (const auto& t : _attrib.vertexTexcoord) {
	//	std::cout << t.x << " " << t.y << std::endl;
	//}

	//std::cout << "faces" << std::endl;
	//std::cout << "positionIndex size: " << _index.positionIndex.size() << std::endl;
	//std::cout << "normalIndex size: " << _index.normalIndex.size() << std::endl;
	//std::cout << "texcoordIndex size: " << _index.texcoordIndex.size() << std::endl;
	//for (int i = 0; i < _index.positionIndex.size(); ++i) {
	//	std::cout << _index.positionIndex[i].x << "/" << _index.texcoordIndex[i].x << "/" << _index.normalIndex[i].x << " ";
	//	std::cout << _index.positionIndex[i].y << "/" << _index.texcoordIndex[i].y << "/" << _index.normalIndex[i].y << " ";
	//	std::cout << _index.positionIndex[i].z << "/" << _index.texcoordIndex[i].z << "/" << _index.normalIndex[i].z << "\n";
	//}

	return true;
}


void SaveObj(attrib_t& _attrib, index_t& _index){

//	const std::string modelSavingPath="./userdata/";
	const std::string _path = "./userdata/writeuser.obj";
	std::ofstream fout(_path);


	if (fout.is_open()){
		for (int i = 0; i < _attrib.vertexPosition.size(); i++) {
			fout << "v " << _attrib.vertexPosition[i].x << " " << _attrib.vertexPosition[i].y << " " << _attrib.vertexPosition[i].z << std::endl;
		}
		for (int i = 0; i < _attrib.vertexTexcoord.size(); i++) {
			fout << "vt " << _attrib.vertexTexcoord[i].x << " " << _attrib.vertexTexcoord[i].y << std::endl;
		}
		for (int i = 0; i < _attrib.vertexNormal.size(); i++) {
			fout << "vn " << _attrib.vertexNormal[i].x << " " << _attrib.vertexNormal[i].y << " " << _attrib.vertexNormal[i].z << std::endl;
		}

		for (int i = 0; i < _index.positionIndex.size(); i++){
			fout << "f " << _index.positionIndex[i].x + 1 << "/" << _index.texcoordIndex[i].x + 1 << "/" << _index.normalIndex[i].x + 1 << " ";
			fout << _index.positionIndex[i].y + 1 << "/" << _index.texcoordIndex[i].y + 1 << "/" << _index.normalIndex[i].y + 1 << " ";
			fout << _index.positionIndex[i].z + 1 << "/" << _index.texcoordIndex[i].z + 1 << "/" << _index.normalIndex[i].z + 1 << std::endl;
		}
		fout.close();
	}
	else {
		std::cout << "file:" << _path << "write error" << std::endl;
	}
}

