#include "Simulation.h"
#include <iostream>

using namespace std;

//Simulation* backup = nullptr;

int main(int argc, char** argv){
    // if(argc!=2){
    //     cout << "usage: simulation <config_path>" << endl;
    //     return 0;
    // }
    // string configurationFile = argv[1];
    // Simulation simulation(configurationFile);
    // simulation.start();
    // if(backup!=nullptr){
    // 	delete backup;
    // 	backup = nullptr;
    // }
    Settlement s1("omri",SettlementType::CITY);
    cout << s1.toString() << endl;
    Facility f1("ido","ido",FacilityCategory::ECONOMY,3,3,3,3);
    cout << f1.toString() << endl;
    f1.setStatus(FacilityStatus:: OPERATIONAL);
    cout << f1.toString() << endl;
    FacilityType ft("aviv",FacilityCategory::LIFE_QUALITY,2,2,2,2);
    Facility f2(ft,"good night");
    cout << f2.toString() << endl;

    return 0;
}