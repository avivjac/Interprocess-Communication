#include "SelectionPolicy.h"
#include <iostream>
#include <algorithm>

using namespace std;

//Naive selection class
NaiveSelection::NaiveSelection(): lastSelectedIndex(0)
{}

NaiveSelection::NaiveSelection(const int index): lastSelectedIndex(index)
{}

const FacilityType& NaiveSelection:: selectFacility(const vector<FacilityType>& facilitiesOptions) {
    int i = lastSelectedIndex;
    lastSelectedIndex = (lastSelectedIndex+1)%facilitiesOptions.size(); //modulo
    return facilitiesOptions[i];
}

const string NaiveSelection:: toString() const {
    return "last selected index = " + to_string(lastSelectedIndex);
}

NaiveSelection* NaiveSelection:: clone() const {
    return &NaiveSelection(lastSelectedIndex);
}

//NaiveSelection::~NaiveSelection() override = default;
