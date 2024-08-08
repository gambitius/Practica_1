/*
 *  Charge Histogram Generator by Charly
 *  Version: 1.2
 *  Author: Carlos Flores Melendez
 *  Date: January 2024
 *  Andres Bello University - SAPHIR
 *  Chile
 *
 * This code is a ROOT macro that generates a Charge Histogram
 * using events stored in a set of txt files.  
 * 
 * In order to use this code, is necessary to run the csvRead.cpp macro first.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "TH1D.h"
#include "TCanvas.h"
#include "ROOT/TThreadExecutor.hxx"

using namespace std;

int chargeHisto() {

    // Reset ROOT
    gROOT->Reset();

    // Enable ROOT's implicit multithreading
    ROOT::EnableImplicitMT();

    // EDITABLE Variables
    //--------------------------------------------------------------------------------------------------------------
    // Folder path
    string filefolder = "/home/martinus/Escritorio/ledchar/3V5_BLUE_LED"; 

    // Number of Events
    size_t numberOfEvents = 256;

    // First Time Point for Histogram:
    double minTimeValue = 3000;

    // Last Time Point for Histogram:
    double maxTimeValue = 4000;

    //--------------------------------------------------------------------------------------------------------------

    // Vectors to store data
    vector<double> timeWindow;
    vector<double> areas;
    vector<double> areasMultiplied;
    vector<double> minVoltages;
    vector<double> maxVoltages;

    // Variables
    bool error = false;
    size_t numColumns = 0;
    size_t lineCount = 0;
    size_t resolution = 0;
    size_t eventNumber = 1;
    int baselinePortion = 0;
    int baselineCount = 0;
    double baseline = 0;
    double timeValue = 0;
    double voltageValue = 0;
    double voltageCorrected = 0;
    double meanBaseline = 0;
    double maxVoltageSum = 0;
    double minVoltageSum = 0;
    double maxVoltageMean = 0;
    double minVoltageMean = 0;
    double globalMinVoltage = numeric_limits<double>::max();
    double globalMaxVoltage = numeric_limits<double>::lowest();
    double minArea = numeric_limits<double>::max();
    double maxArea = numeric_limits<double>::lowest();
    const double chargeMultiplier = 1000000000000.0; // Default value is 1000000000000.0 for pico coulombs
    const double timeMultiplier = 1000000.0;    // Default value is 1000000.0 for microseconds
    cout << " " << endl;
    cout << "   --- Charge Histogram Generator by Charly ---    " << endl;
    cout << " " << endl;

    // Read Time File ----------------------------------------------------------------------------------------------
    string timeWindowFilename = (filefolder + "/txt/Time_Window.txt");
    ifstream timeWindowFile(timeWindowFilename);
    if (!timeWindowFile.is_open()) {
        cerr << "-Error: Could not open file " << timeWindowFilename << endl;
        error = true;
        return 1;
    } else {
        cout << "Reading " << timeWindowFilename << " ..." << endl;
    }

    // Read Time Data
    while (timeWindowFile >> timeValue) {
        timeWindow.push_back(timeValue);
        lineCount++;
        //cout << timeValue << endl;
    }

    // Set resolution
    resolution = lineCount;
    cout << "Time window reading OK" << endl;
    cout << "- Resolution: " << resolution << endl;
    lineCount = 0;

    if (maxTimeValue > resolution){
        cerr << " - ERROR - maxTimeValue > resolution" << endl;
        error = true;
        return 2;
    } else if (minTimeValue <= 0){
        cerr << " - ERROR - Invalid minTimeValue" << endl;
        error = true;
        return 3;
    } else {
        // Time Calculations ---------------------------------------------------------------------------------------

        // Calculate Delta T
        double deltaT = timeWindow.back() - timeWindow[timeWindow.size() - 2];
        cout << "- Delta T: " << deltaT << endl;

        // Calculate constant factor with 50 ohm
        double constantFactor = deltaT/50.0;
        cout << "- Constant factor: " << constantFactor << endl;

        // Calculate portion of baseline to obtain meanBaseline
        baselinePortion = static_cast<int>(round((resolution*10)/100)); // Portion of 10%
        cout << "- Baseline portion: " << baselinePortion << " points" << endl;

        // Read txt Event Files ------------------------------------------------------------------------------------
        cout << " " << endl;
        cout << "Reading events in " << filefolder + "/txt/" << " ..." << endl;
        for(eventNumber = 1; eventNumber <= numberOfEvents; ++eventNumber){
            double minVoltage = numeric_limits<double>::max();
            double maxVoltage = numeric_limits<double>::lowest();
            string eventFilename = (filefolder + "/txt/Event" + to_string(eventNumber) + ".txt");
            ifstream eventFile(eventFilename);
            double area = 0;
            if (!eventFile.is_open()) {
                cerr << " - ERROR - Could not open file " << eventFilename << endl;
                error = true;
                return 4;
            } else {
                //cout << "Reading txt file: " << eventFilename << " ...";
            }
            
            // Read Voltage Data
            while (eventFile >> voltageValue) {
                // Store Baseline Values
                if(lineCount <= baselinePortion){
                    baseline = baseline + voltageValue;
                    baselineCount++;

                    // Calculate meanBaseline    
                    if(lineCount == baselinePortion){
                        meanBaseline = baseline/baselineCount;
                    }

                } else {
                    // Read pulse voltage points and calculate area
                    if(lineCount >= minTimeValue && lineCount <= maxTimeValue){
                        voltageCorrected = voltageValue - meanBaseline;
                        maxVoltage = max(maxVoltage, voltageCorrected);
                        minVoltage = min(minVoltage, voltageCorrected);
                        area = area + voltageCorrected;
                    }
                }
                lineCount++;
                //cout << voltageValue << endl;
            }
            // Line Count Error check
            if(lineCount != resolution){
                areas.push_back(0);
                cerr << " - ERROR - Line count not match in " << eventFilename << endl;
                cerr << "Line count: " << lineCount << endl;
                error = true;
                return 5;
            } else {
                // Store area behind the curve of current event
                areas.push_back(area*-constantFactor);
                maxVoltages.push_back(maxVoltage);
                minVoltages.push_back(minVoltage);
                //cout << "Event " << eventNumber <<  " reading OK" << endl;
            }
            lineCount = 0;
        
            //Print progress
            if (eventNumber % 1 == 0) {
                cout << "\r- Events Processed: " << eventNumber;
                cout.flush();
            }
        }
    }

    // Charge Histogram --------------------------------------------------------------------------------------------

    // Calculate bin number
    cout << " " << endl;
    cout << " " << endl;
    cout << "Calculating bin number..." << endl;
    int binNumber = 7*sqrt(areas.size());
    cout << "- Bin number: " << binNumber << endl;
    cout << " " << endl;
    cout << "Creating Histogram..." << endl;

    // Unit Conversion of areas
    for (const double value : areas) {
        areasMultiplied.push_back(value * chargeMultiplier);
    }

    // Calculate min and max values of areas
    for (const double areaValue : areasMultiplied){
        minArea = min(minArea, areaValue);
        maxArea = max(maxArea, areaValue);
    }

    // Create a TCanvas
    TCanvas *canvas = new TCanvas("canvas", "Charge Histogram", 1920, 1080);

    // Set Grid
    canvas->SetGrid();

    // New Histrogram
    auto h1 = new TH1D("Charge", ("Charge Histogram - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str(), binNumber, 0, maxArea*1.05);
    
    // Fill the histogram with voltage data of all events
    for (double areaValue : areasMultiplied) {
        h1->Fill(areaValue);
    }

    // Draw the histogram on the canvas
    h1->Draw();

    // Set axis label
    h1->GetXaxis()->SetTitle(("Picocoulombs"));

    // Save the canvas as a PNG file
    string pngFilename;
    pngFilename = filefolder + "/images/Charge_Histogram_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas->SaveAs(pngFilename.c_str());

    // Max and Min Voltages Files ----------------------------------------------------------------------------------

    cout << " " << endl;
    cout << "Creating Max and Min Voltage txt files..." << endl;

    // Calculate average minimum and maximum voltage across events
    for (const double value : minVoltages) {
        minVoltageSum = minVoltageSum + value;
    }
    minVoltageMean = minVoltageSum/(minVoltages.size());
                    
    for (const double value : maxVoltages) {
        maxVoltageSum = maxVoltageSum + value;
    }                
    maxVoltageMean = maxVoltageSum/(maxVoltages.size());

    // Calculate Absolute Maximum and Minimum
    for (const double value : minVoltages) {
        globalMinVoltage = min(globalMinVoltage, value);
    }
    for (const double value : maxVoltages) {
        globalMaxVoltage = max(globalMaxVoltage, value);
    } 
    
    // Print averages and absoluts Max and Mins
    cout << "- Average Min Voltage: " << minVoltageMean << endl;
    cout << "- Average Max Voltage:  " << maxVoltageMean << endl;
    cout << "- Global Min Voltage:  " << globalMinVoltage << endl;
    cout << "- Global Max Voltage:   " << globalMaxVoltage << endl;
    cout << " " << endl;
    
    // Open a file for writing minVoltages data
    ofstream minVoltagesFile(filefolder + "/txt/MinVoltages.txt");
    if (!minVoltagesFile.is_open()) {
        cerr << " - ERROR - Could not open file for writing MinVoltages data" << endl;
        error = true;
        return 6;
    }

    // Write minVoltages file
    minVoltagesFile << "- SUMMARY -----------------------------------------------------" << endl;
    minVoltagesFile << "Time Window in microseconds: " << (timeWindow[minTimeValue - 1])*timeMultiplier << " to " << (timeWindow[maxTimeValue - 1])*timeMultiplier << " microseconds" << endl;
    minVoltagesFile << "Time Window in points: " << minTimeValue << " to " << maxTimeValue << " points" << endl;
    minVoltagesFile << "Average Minimun Voltage: " << minVoltageMean << endl;
    minVoltagesFile << "Global Minimum Voltage:  " << globalMinVoltage << endl;
    minVoltagesFile << " " << endl;
    minVoltagesFile << "- MIN VOLTAGE VALUES ------------------------------------------" << endl;
    for (const double value : minVoltages) {
        // Write maxVoltage data
        minVoltagesFile << value << endl;
    }

    // Close the minVoltages file
    minVoltagesFile.close();

    // Open a file for writing maxVoltages data
    ofstream maxVoltagesFile(filefolder + "/txt/MaxVoltages.txt");
    if (!maxVoltagesFile.is_open()) {
        cerr << " - ERROR - Could not open file for writing MaxVoltages data" << endl;
        error = true;
        return 7;
    }

    // Write maxVoltages file
    maxVoltagesFile << "- SUMMARY -----------------------------------------------------" << endl;
    maxVoltagesFile << "Time Window in microseconds: " << (timeWindow[minTimeValue - 1])*timeMultiplier << " to " << (timeWindow[maxTimeValue - 1])*timeMultiplier << " microseconds" << endl;
    maxVoltagesFile << "Time Window in points: " << minTimeValue << " to " << maxTimeValue << " points" << endl;
    maxVoltagesFile << "Average Maximum Voltage: " << maxVoltageMean << endl;
    maxVoltagesFile << "Global Maximum Voltage:  " << globalMaxVoltage << endl;
    maxVoltagesFile << " " << endl;
    maxVoltagesFile << "- MAX VOLTAGE VALUES ------------------------------------------" << endl;
    for (const double value : maxVoltages) {
        // Write maxVoltage data
        maxVoltagesFile << value << endl;
    }

    // Close the maxVoltages file
    maxVoltagesFile.close();

    // Create a ROOT File with the histogram -----------------------------------------------------------------------
    cout << "Creating ROOT File..." << endl;

    // Create TFile
    string rootFilename;
    rootFilename = filefolder + "/" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".root";
    auto *f = new TFile (rootFilename.c_str(),  "RECREATE");

    // Save TFile
    h1->Write();
    f->Write();
    f->Close();

    // END ---------------------------------------------------------------------------------------------------------
    if (!error) {
        cout << "Done!" << endl;
        cout << " " << endl;
    }

    return 0;
}
