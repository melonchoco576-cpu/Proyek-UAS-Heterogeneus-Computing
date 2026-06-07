#pragma once
#include "data_model.h"
#include <string>
#include <vector>

// Membaca dan memvalidasi dataset CSV
bool baca_dataset(const std::string& path, std::vector<RoomData>& data);
