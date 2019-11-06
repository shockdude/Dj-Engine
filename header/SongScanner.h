#pragma once
#include "SimpleINI/SimpleIni.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <list>

struct SongEntry {
	std::string path;
	std::string s1;
	std::string s2;
	std::string a1;
	std::string a2;
	std::string charter;
	std::string mixer;
	int dTrack;
	int dTap;
	int dCrossfade;
	int dScratch;
};

class SongScanner
{
public:
	static void load(const std::string& root, std::vector<SongEntry>& list);
};
