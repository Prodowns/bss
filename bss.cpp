#include "json.hpp"
#include <iostream>
#include <fstream>
using namespace std;
using json = nlohmann::json;

double distance(double lon1, double lat1, double lon2, double lat2);

int usage() {cerr << "arguments: -c TUR -t border/cable" << endl; return -1;};

int main(int argc, char const *argv[])
{
	bool borderOrCable = false;
	string countryIso;

	switch(argc)
	{
		case 5:
		{
			string bc(argv[4]);
			if(bc == "cable")
				borderOrCable = true;
			else if(bc != "border")
				return usage();
		}
		case 4:
			if(string(argv[3]) != "-t")
				return usage();
		case 3:
			countryIso = argv[2];
		case 2:
			if(string(argv[1]) != "-c")
				return usage();
			break;
		default:
			if(argc > 5)
				return usage();
			break;
	}

	ifstream countriesFile("countries.geojson");
	json countries;
	countriesFile >> countries;

	ifstream capitalsFile("capitals.geojson");
	json capitals;
	capitalsFile >> capitals;

	json result;

	if(!countryIso.empty())
	{
		bool countryFound = false;
		for (const auto & country : countries["features"])
		{
			if (country["properties"]["ISO_A3"] == countryIso)
			{
				countryFound = true;

				double capitalLongitude = 0;
				double capitalLatitude = 0;

				if(borderOrCable)
				{
					bool found = false;
					for (const auto & capital : capitals["features"])
					{
						if (capital["properties"]["iso3"] == countryIso)
						{
							found = true;
							capitalLongitude = capital["geometry"]["coordinates"][0].get<double>();
							capitalLatitude = capital["geometry"]["coordinates"][1].get<double>();
							break;
						}
					}
					if (!found)
					{
						cerr << "Capital was not found for " << countryIso << endl;
						cerr << "Calculating only border length" << endl;
						borderOrCable = false;
					}
				}

				double minDistance = 40000;
				double sum = 0;

				if(country["geometry"]["type"] == "Polygon")
				{
					bool first = true;
					double lon0 = 0, lat0 = 0;
					for (const auto & coord : country["geometry"]["coordinates"][0])
					{
						if(first)
						{
							lon0 = coord[0].get<double>();
							lat0 = coord[1].get<double>();
							if(borderOrCable)
								minDistance = distance(capitalLongitude, capitalLatitude, lon0, lat0);
							first = false;
						}

						double lon1 = coord[0].get<double>();
						double lat1 = coord[1].get<double>();
						sum += distance(lon1, lat1, lon0, lat0);
						lon0 = lon1;
						lat0 = lat1;

						if (borderOrCable)
						{
							double newDistance = distance(capitalLongitude, capitalLatitude, lon0, lat0);
							if(newDistance < minDistance)
								minDistance = newDistance;
						}
					}
				}
				else if(country["geometry"]["type"] == "MultiPolygon")
				{
					for (const auto & polygon : country["geometry"]["coordinates"])
					{
						bool first = true;
						double lon0 = 0, lat0 = 0;
						for (const auto & coord : polygon[0])
						{
							if(first)
							{
								lon0 = coord[0].get<double>();
								lat0 = coord[1].get<double>();
								if(borderOrCable)
									minDistance = distance(capitalLongitude, capitalLatitude, lon0, lat0);
								first = false;
							}

							double lon1 = coord[0].get<double>();
							double lat1 = coord[1].get<double>();
							sum += distance(lon1, lat1, lon0, lat0);
							lon0 = lon1;
							lat0 = lat1;

							if (borderOrCable)
							{
								double newDistance = distance(capitalLongitude, capitalLatitude, lon0, lat0);
								if(newDistance < minDistance)
									minDistance = newDistance;
							}
						}
					}
				}

				if (borderOrCable)
				{
					sum += 2 * minDistance; // twice cable length capital to nearest border point
				}

				result[0]["name"] = country["properties"]["ADMIN"];
				result[0]["iso_a3"] = country["properties"]["ISO_A3"];
				if (borderOrCable)
					result[0]["cable"] = sum;
				else
					result[0]["border"] = sum;

				cout << result << endl;

				return 0;
			}
		}

		if(countryFound == false)
		{
			cerr << "iso_a3 code " << countryIso << " was not found in countries.geojson file." << endl;
			return -1;
		}
	}
	else
	{
		int i = 0;
		for (const auto & country : countries["features"])
		{
			double sum = 0;

			if(country["geometry"]["type"] == "Polygon")
			{
				bool first = true;
				double lon0 = 0, lat0 = 0;
				for (const auto & coord : country["geometry"]["coordinates"][0])
				{
					if(first)
					{
						lon0 = coord[0].get<double>();
						lat0 = coord[1].get<double>();
						first = false;
					}

					double lon1 = coord[0].get<double>();
					double lat1 = coord[1].get<double>();
					sum += distance(lon1, lat1, lon0, lat0);
					lon0 = lon1;
					lat0 = lat1;
				}
			}
			else if(country["geometry"]["type"] == "MultiPolygon")
			{
				for (const auto & polygon : country["geometry"]["coordinates"])
				{
					bool first = true;
					double lon0 = 0, lat0 = 0;
					for (const auto & coord : polygon[0])
					{
						if(first)
						{
							lon0 = coord[0].get<double>();
							lat0 = coord[1].get<double>();
							first = false;
						}

						double lon1 = coord[0].get<double>();
						double lat1 = coord[1].get<double>();
						sum += distance(lon1, lat1, lon0, lat0);
						lon0 = lon1;
						lat0 = lat1;
					}
				}
			}

			result[i]["name"] = country["properties"]["ADMIN"];
			result[i]["iso_a3"] = country["properties"]["ISO_A3"];
			result[i]["border"] = sum;
			i++;
		}
	}

	cout << result << endl;

	return 0;
}

/**
 * Calculate the approximate distance between two coordinates (lat/lon)
 *
 * © Chris Veness, MIT-licensed,
 * http://www.movable-type.co.uk/scripts/latlong.html#equirectangular
 */
const double PI = 3.141592653589793238463;
double distance(double lon1, double lat1, double lon2, double lat2)
{
    int R = 6371;
    double deltaLon = (lon2 - lon1) * PI / 180;
    lat1 = lat1 * PI / 180;
    lat2 = lat2 * PI / 180;
    double x = deltaLon * cos((lat1+lat2)/2);
    double y = (lat2-lat1);
    double d = sqrt(x*x + y*y);
    return R * d;
};
