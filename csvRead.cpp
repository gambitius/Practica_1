/*
 *  Keysight CSV Reader by Charly
 *  Version: 1.2
 *  Author: Carlos Flores Melendez
 *  Date: January 2024
 *  Andres Bello University - SAPHIR
 *  Chile
 *
 * This code is a ROOT macro that reads a .csv file from Keysight oscilloscope,  
 * then generates a TXT file and a PNG plot per each event found in the .csv file.
 * And finally provide a histogram of the baseline voltage to find deviation from zero. 
 * 
 * In order to use this code, is necessary to put the .csv file inside a folder with 
 * the same name of the file, and then, inside this folder, create two folders;
 * one folder called "images", and other called "txt".
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <limits>     // Added for std::numeric_limits
#include <algorithm>  // Added for std::min
#include <TImage.h>
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMarker.h"
#include "ROOT/TThreadExecutor.hxx"

using namespace std;

// Just for record time of execution
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

// Folder Path
string filefolder = "C:/root_v6.28.06/macros/Data/R7600U_others/800V/800V_3_055V_66mV_6ns_500kHz"; // <-- EDIT THIS --

//Global Variables
double globalMinX = numeric_limits<double>::max();
double globalMaxX = numeric_limits<double>::lowest();
double globalMinY = numeric_limits<double>::max();
double globalMaxY = numeric_limits<double>::lowest();
double firstValue = 0;
double lastValue = 0;
double timeWindow = 0;
const double timeDataMultiplier = 1000000.0;    // Default value is 1000000.0 for microseconds
bool error = false;
size_t numColumns = 0;
size_t selectedPair = 1;
size_t possibleXColumns = 0;
string filename = filefolder + "/" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".csv";  // Path to CSV file

// Function to read CSV file and extract all columns --------------------------------------------------------------------------------------
size_t readCSV(string filename, vector<vector<double>>& data) {

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        error = true;
        return 1;
    }

    string line;

    // Skip the first 25 lines
    for (size_t i = 0; i < 25; ++i)
        getline(file, line);

    // Read the first line to determine the number of columns
    getline(file, line);
    istringstream header(line);
    double value;
    size_t lineCount = 1;

    // Travel first row
    while (header >> value) {
        // Create new inner vector
        data.push_back(vector<double>());
        // Store value in the last inner vector 
        data.back().push_back(value);
        numColumns++;
        if (header.peek() == ','){
            header.ignore();
        }
    }

    // Read the rest of the file
    cout << "Reading CSV file in " << filename << endl;

    while (getline(file, line)) {
        istringstream iss(line);
        size_t colIndex = 1; // Start column indexing from 1

        while (iss >> value) {
            data[colIndex - 1].push_back(value); // Adjust column index

            // Update global min/max values for X and Y axes
            if (colIndex % 2 == 1) {  // X-axis columns
                globalMinX = min(globalMinX, value);
                globalMaxX = max(globalMaxX, value);
            } else {  // Y-axis columns
                globalMinY = min(globalMinY, value);
                globalMaxY = max(globalMaxY, value);
            }

            // Move to the next column
            if (iss.peek() == ',')
                iss.ignore();

            colIndex++;
        }

        lineCount++;

        // Display progress every 1000 lines
        if (lineCount % 100 == 0) {
            cout << "\rReading: " << lineCount << " lines";
            cout.flush();
        }
    }
    cout << "\rReading: " << " ... Done.                  " << endl;
    cout << "\rRead " << lineCount << " lines and " << numColumns << " columns." << endl;
    cout << " " << endl;

    return lineCount;
}

// Function to plot a single graph --------------------------------------------------------------------------------------------------------
void plotSelectedXAndAutoY(const vector<vector<double>>& data, const vector<vector<double>>& dataMultiplied, size_t resolution) {

    // Calculate the corresponding X and Y axis columns
    size_t selectedXAxis = (selectedPair - 1) * 2;
    size_t selectedYAxis = selectedXAxis + 1;

    // Create a TCanvas
    TCanvas *canvas = new TCanvas("canvas", "CSV Data Plot", 1920, 1080);

    // Set Grid
    canvas->SetGrid();

    // Create a TGraph for the selected pair of columns
    TGraph *graph = new TGraph(resolution, &dataMultiplied[0][0], &data[selectedYAxis][0]);

    // Set the range for X and Y axes
    graph->GetXaxis()->SetRangeUser(firstValue, lastValue);
    graph->GetYaxis()->SetRangeUser(globalMinY, globalMaxY);

    graph->SetLineWidth(3);
    graph->SetLineColor(4);

    // Draw the TGraph
    graph->Draw("APL"); // A: axes, P: marker, L: line

    // Set axis labels
    graph->GetXaxis()->SetTitle(("Microseconds"));
    graph->GetYaxis()->SetTitle(("Volts"));

    // Set graph title with file number
    graph->SetTitle(("Event " + to_string(selectedPair) + " - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str());

    // Save the canvas as a PNG file
    string pngFilename = filefolder + "/images/Event_" + to_string(selectedPair) + "_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas->SaveAs(pngFilename.c_str());

    // Clean up
    delete graph;
    delete canvas;
}

// Function to plot all graphs overlapped with seconds in the X axis ---------------------------------------------------------------------
void plotAllGraphsOverlapped(const vector<vector<double>>& data, const vector<vector<double>>& dataMultiplied, size_t resolution) {

    // Create a TCanvas
    TCanvas *canvas1 = new TCanvas("canvas1", "Graphs", 1920, 1080);

    // Set Grid
    canvas1->SetGrid();

    // Create a TMultiGraph to hold all the individual TGraphs
    auto *mg = new TMultiGraph();

    for (selectedPair = 1; selectedPair <= possibleXColumns; ++selectedPair) {
        
        //Print progress
        if (selectedPair % 1 == 0) {
            cout << "\rEvents: " << selectedPair;
            cout.flush();
        }
        
        // Calculate the corresponding X and Y axis columns
        size_t selectedXAxis = (selectedPair - 1) * 2;
        size_t selectedYAxis = selectedXAxis + 1;

        // Create a TGraph for the selected pair of columns
        TGraph *graph = new TGraph(resolution, &dataMultiplied[0][0], &data[selectedYAxis][0]);

        // Set the range for X and Y axes
        graph->GetXaxis()->SetRangeUser(firstValue, lastValue);
        graph->GetYaxis()->SetRangeUser(globalMinY, globalMaxY);

        // Add the TGraph to the TMultiGraph
        mg->Add(graph); 
    }

    // Draw the TMultiGraph on the canvas
    mg->Draw("A PMC PLC");

    // Set Title
    mg->SetTitle(("All Events - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str());

    // Set axis labels
    mg->GetXaxis()->SetTitle(("Microseconds"));
    mg->GetYaxis()->SetTitle(("Volts"));

    // Save the canvas as a PNG file
    cout << " " << endl;
    string pngFilename = filefolder + "/images/All_Events_In_Seconds_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas1->SaveAs(pngFilename.c_str());

    // Clean up
    delete mg;
    delete canvas1;
}

// Function to plot all graphs overlapped with points in the X axis -----------------------------------------------------------------------
void plotAllGraphsOverlapped2(const vector<vector<double>>& data, const vector<double>& timeResolutionVect, size_t resolution) {

    //Mode for ROOT graphics
    //gROOT->SetBatch(kFALSE);  // Set to kTRUE to run in batch mode (no GUI)

    // Create a TCanvas
    TCanvas *canvas2 = new TCanvas("canvas2", "Graphs", 1920, 1080);

    // Set Grid
    canvas2->SetGrid();

    // Create a TMultiGraph to hold all the individual TGraphs
    auto *mg = new TMultiGraph();

    for (selectedPair = 1; selectedPair <= possibleXColumns; ++selectedPair) {
        
        //Print progress
        if (selectedPair % 1 == 0) {
            cout << "\rEvents: " << selectedPair;
            cout.flush();
        }
        
        // Calculate the corresponding X and Y axis columns
        size_t selectedXAxis = (selectedPair - 1) * 2;
        size_t selectedYAxis = selectedXAxis + 1;

        // Create a TGraph for the selected pair of columns
        TGraph *graph = new TGraph(resolution, &timeResolutionVect[0], &data[selectedYAxis][0]);

        // Set the range for X and Y axes
        graph->GetXaxis()->SetRangeUser(firstValue, lastValue);
        graph->GetYaxis()->SetRangeUser(globalMinY, globalMaxY);

        // Add the TGraph to the TMultiGraph
        mg->Add(graph); 
    }

    // Draw the TMultiGraph on the canvas
    mg->Draw("A PMC PLC");

    // Set Title
    mg->SetTitle(("All Events - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str());

    // Set axis labels
    mg->GetXaxis()->SetTitle(("Points"));
    mg->GetYaxis()->SetTitle(("Volts"));

    // Save the canvas as a PNG file
    cout << " " << endl;
    string pngFilename = filefolder + "/images/All_Events_In_Points_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas2->SaveAs(pngFilename.c_str());

    // Clean up
    delete mg;
    delete canvas2;
}

// Function to plot Voltage Histogram -----------------------------------------------------------------------------------------------------
int voltageHistogram(size_t resolution){

    //Mode for ROOT graphics
    gROOT->SetBatch(kFALSE);  // Set to kTRUE to run in batch mode (no GUI)

    int baselinePortion = 0;
    int baselineCount = 0;
    double baseline = 0;
    double timeValue = 0;
    double voltageValue = 0;
    double minTimeValue = 1;
    double maxTimeValue = 0;
    double minVoltage = numeric_limits<double>::max();
    double maxVoltage = numeric_limits<double>::lowest();
    size_t lineCount = 0;
    size_t eventNumber = 1;

    vector<double> voltages;

    // Calculate portion of baseline to plot
    baselinePortion = static_cast<int>(round((resolution*10)/100)); // Portion of 10%
    cout << "Baseline portion: " << baselinePortion << endl;
    maxTimeValue = baselinePortion;

    // Open file stream for every Event file and read data 
    cout << " " << endl;
    cout << "Reading events in " << filefolder << " ..." << endl;
    for(eventNumber = 1; eventNumber <= possibleXColumns; ++eventNumber){
        string eventFilename = (filefolder + "/txt/Event" + to_string(eventNumber) + ".txt");
        ifstream eventFile(eventFilename);
        if (!eventFile.is_open()) {
            cerr << "Error: Could not open file " << eventFilename << endl;
            error = true;
            return 4;
        } else {
            //cout << "Reading txt file: " << eventFilename << " ...";
        }

        // Read Voltage Data
        while (eventFile >> voltageValue) {
            if(lineCount >= minTimeValue && lineCount <= maxTimeValue){
                voltages.push_back(voltageValue);
                minVoltage = min(minVoltage, voltageValue);
                maxVoltage = max(maxVoltage, voltageValue);
            }
            lineCount++;
            //cout << voltageValue << endl;
        }

        // Error check
        if(lineCount != resolution){
            cerr << "Error: Line count not match in " << eventFilename << endl;
            cerr << "Line count: " << lineCount << endl;
            error = true;
            return 5;
        } else {
            //cout << "Event " << eventNumber <<  " reading OK" << endl;
        }
        lineCount = 0;

        //Print progress
        if (eventNumber % 1 == 0) {
            cout << "\rEvents Processed: " << eventNumber;
            cout.flush();
        }
    }
    if(!error){
        cout << " " << endl;
        cout << "Events reading OK" << endl;
        cout << " " << endl;
    }

     // Calculate bin number
    size_t vectorSize = voltages.size();
    int binNumber = 7*sqrt(vectorSize);
    cout << "Bin number: " << binNumber << endl;
    cout << " " << endl;

    // Create a TCanvas
    TCanvas *canvas3 = new TCanvas("canvas3", "Voltage Histogram", 1920, 1080);

    // Set Grid
    canvas3->SetGrid();
    
    // Create Histogram 
    auto h1 = new TH1D("Voltage", ("Baseline Voltage - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str(), binNumber, minVoltage, maxVoltage);
    
    // Fill the histogram with voltage data
    for (double value : voltages) {
        h1->Fill(value);
    }

    // Draw the histogram on the canvas
    h1->Draw();

    // Set axis label
    h1->GetXaxis()->SetTitle(("Volts"));

    // Save the canvas as a PNG file
    string pngFilename;
    pngFilename = filefolder + "/images/Baseline_Voltage_Histogram_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas3->SaveAs(pngFilename.c_str());

    // Clean up
    //delete h1;
    //delete canvas3;
    return 0;
}

// Main function --------------------------------------------------------------------------------------------------------------------------
int csvRead() {

    // Record the start time
    auto start = high_resolution_clock::now();

    // Enable ROOT's implicit multithreading
    ROOT::EnableImplicitMT();

    //Mode for ROOT graphics
    gROOT->SetBatch(kTRUE);  // Set to kTRUE to run in batch mode (no GUI)

    // Welcome message
    cout << " " << endl;
    cout << "   ***   Welcome to the Keysight CSV Reader by Charly!   ***   " << endl;
    cout << " " << endl;

    // Vectors to store data
    vector<double> timeResolutionVect;
    vector<double> timeData;
    vector<vector<double>> data;
    vector<vector<double>> dataMultiplied;

    // Read data from CSV (skipping the first 25 lines)
    size_t resolution = readCSV(filename, data);
    if (data.empty()) {
        cerr << "Error: No data read from CSV file." << endl;
        error = true;
        return 6;
    }

    // Check if data vector is not empty and write time window file
    cout << "Creating txt file for time window..." << endl;
    if (!data.empty() && !data[0].empty()) {
        timeData = data[0];
        ofstream outputFile(filefolder + "/txt/Time_Window.txt");
        if (!outputFile.is_open()) {
            cerr << "Error: Could not create or open output file " << "Time_Window.txt" << endl;
            error = true;
            return 7;
        }

        // Write the data to the file
        for (size_t i = 0; i < timeData.size(); ++i) {
            outputFile << timeData[i] << endl;
        }

        cout << "Saved data for time window in Time_Window.txt" << endl;
        cout << " " << endl;
    }

    // Write odd-numbered columns to separate event files
    cout << "Creating txt files per event..." << endl;
    for (size_t colIndex = 1; colIndex < numColumns; colIndex += 2) {
        ofstream outputFileEvent(filefolder + "/txt/Event" + to_string(colIndex / 2 + 1) + ".txt");
        if (!outputFileEvent.is_open()) {
            cerr << "Error: Could not create or open output file for Event " << colIndex / 2 + 1 << endl;
            error = true;
            return 8;
        }

        // Write the data for the current event to the file
        for (const auto& value : data[colIndex]) {
            outputFileEvent << value << endl;
        }

        //Print progress
        cout << "\rEvents: " << colIndex / 2 + 1;
        cout.flush();
    }
    cout << " " << endl;

    // Calculate the possible number of X-axis columns
    possibleXColumns = numColumns / 2;

    // Adjust time and volt values by timeDataMultiplier
    for (const auto& row : data) {
        vector<double> multipliedRow;
        // Multiply each element by the timeDataMultiplier
        for (double value : row) {
            multipliedRow.push_back(value * timeDataMultiplier);
        }
        // Add the multiplied row to the dataMultiplied vector of vectors
        dataMultiplied.push_back(multipliedRow);
    }

    // Calculate Time Window
    firstValue = dataMultiplied[0].front();  // Read the first value of first column
    lastValue = dataMultiplied[0].back();    // Read the last value of first column
    timeWindow = lastValue - firstValue;

/*
    // Plotting individual graphs
    cout << " " << endl;
    cout << "Making plots... " << endl;
    for (selectedPair = 1; selectedPair <= possibleXColumns; ++selectedPair) {
        // Plot a graph with selected X and automatically selected Y axis
        plotSelectedXAndAutoY(data, dataMultiplied, resolution);
    }
*/
    // Plotting all graphs overlapped with time axis in microseconds
    cout << " " << endl;
    cout << "Making plot of all events overlapped... " << endl;
    selectedPair = 1;
    plotAllGraphsOverlapped(data, dataMultiplied, resolution);
   
    // Plotting all graphs overlapped with time axis in points of time resolution
    cout << " " << endl;
    cout << "Making second plot of all events overlapped... " << endl;
    for (int i = 1; i <= resolution; ++i) {
        timeResolutionVect.push_back(i);
    }
    selectedPair = 1;
    plotAllGraphsOverlapped2(data, timeResolutionVect, resolution);

    // Print voltage histogram of baseline
    cout << " " << endl;
    cout << "Making baseline voltage histogram... " << endl;
    voltageHistogram(resolution);

    // Print summary
    cout << " " << endl;
    cout << "- SUMMARY ---------------------------------------------------------------------------------------------" << endl;
    cout << " " << endl;
    cout << "- Number of events: " << possibleXColumns << " events."<< endl;
    cout << "- Resolution: " << resolution << " points."<< endl;
    cout << " " << endl;
    cout << "- Time window first Value: " << firstValue << " microseconds." << endl;
    cout << "- Time window Last Value:   " << lastValue << " microseconds." << endl;
    cout << "- Time window lenght:       " << timeWindow << " microseconds." << endl;
    cout << " " << endl;
    cout << "- All events time window first value: " << firstValue << " microseconds." << endl;
    cout << "- All events time window last value:   " << globalMaxX << " seconds." << endl;
    cout << " " << endl;
    cout << "- Minimun voltage: " << globalMinY << " volts." << endl;
    cout << "- Maximum voltage:  " << globalMaxY << " volts." << endl;
    cout << " " << endl;
    cout << "-------------------------------------------------------------------------------------------------------" << endl;

    // Record the end time
    auto end = high_resolution_clock::now();
    // Calculate the duration
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Time taken by code: " << duration.count()/1000000.0 << " seconds or " << (duration.count()/1000000.0)/60.0 << " minutes. "<< endl;

    return 0;
}