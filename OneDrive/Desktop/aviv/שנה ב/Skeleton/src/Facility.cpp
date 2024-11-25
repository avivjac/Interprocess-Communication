#include "Facility.h"
#include <iostream>
#include <algorithm>

using namespace std;

 std::string categoryToString(FacilityCategory category) {
    switch (category) {
        case FacilityCategory::LIFE_QUALITY :    return "Life quality";
        case FacilityCategory::ECONOMY:    return "Economy";
        case FacilityCategory::ENVIRONMENT: return "Environment";
        default:            return "Unknown";
    }
}

std::string statusToString(FacilityStatus status) {
    switch (status) {
        case FacilityStatus::UNDER_CONSTRUCTIONS :    return "Under Constructions";
        case FacilityStatus::OPERATIONAL:    return "Operational";
        default:            return "Unknown";
    }
}

//Facility-Type
//Constructor
FacilityType:: FacilityType(const string &name, const FacilityCategory category, const int price, const int lifeQuality_score, const int economy_score, const int environment_score):name(name),category(category),price(price),lifeQuality_score(lifeQuality_score),economy_score(economy_score),environment_score(environment_score) {
}

//copy constructor
FacilityType::FacilityType(const FacilityType& type): name(type.name),category(type.category),price(type.price),lifeQuality_score(type.lifeQuality_score),economy_score(type.economy_score),environment_score(type.environment_score){
}

const string& FacilityType::getName() const {
    return name;
}

int FacilityType::getCost() const {
    return price;
}

int FacilityType::getLifeQualityScore() const {
    return lifeQuality_score;
}

int FacilityType:: getEnvironmentScore() const {
    return environment_score;
}
int  FacilityType:: getEconomyScore() const {
    return economy_score;
}

FacilityCategory  FacilityType:: getCategory() const {
    return category;
}


//Facility
Facility:: Facility(const string &name, const string &settlementName, const FacilityCategory category, const int price, const int lifeQuality_score, const int economy_score, const int environment_score):FacilityType(name,category,price,lifeQuality_score,economy_score,environment_score),settlementName(settlementName),status(FacilityStatus::UNDER_CONSTRUCTIONS),timeLeft(price) {
 }

Facility::Facility(const FacilityType &type, const string &settlementName): FacilityType(type), settlementName(settlementName),  status(FacilityStatus::UNDER_CONSTRUCTIONS), timeLeft(price){
}
const string & Facility:: getSettlementName() const {
    return settlementName;
}
const int Facility:: getTimeLeft() const {
    return timeLeft;
}

void  Facility:: setStatus(FacilityStatus status) {
    this->status = status;
}
const FacilityStatus&  Facility:: getStatus() const {
    return status;
}

const string Facility:: toString() const {
    string str = "name:" + name + " settlement name:" + settlementName + " category:" + categoryToString(category) + " price:" + to_string(price) + " Life quality score: " + to_string(lifeQuality_score) + " economy score: " + to_string(economy_score) +  " environment score: " + to_string(environment_score) + " Time left: " + to_string(timeLeft) + " Status: " + statusToString(status);
    return str;
}

//FacilityStatus step();