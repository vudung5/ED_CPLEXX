#include <iostream>  
#include <vector>  
#include <cmath>  
#include <chrono>  
#include <numeric>  
#include <algorithm>  
#include <ilcplex/ilocplex.h>
#include "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\Code\\TestCPlexUC\\CSVReader.h"
#include "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\Code\\TestCPlexUC\\matrixCalc.h"

ILOSTLBEGIN

using namespace std;
using namespace std::chrono;

///* ---------- nhân 1 giá trị 15 lần (giống repeatStatus) ---------- */
//template <class T>
//std::vector<T> repeat15to1(const std::vector<T>& in15)
//{
//	std::vector<T> out;
//	out.reserve(in15.size() * rep15to1);         // = 15
//	for (T v : in15)
//		out.insert(out.end(), rep15to1, v);      // 15 phần tử = v
//	return out;
//}
//
///* ---------- cắt chuỗi về [ED_min … 1439] ---------- */
//template <class T>
//std::vector<T> slice1day(std::vector<T>& vec)
//{
//	if (vec.size() < EndIndex1 + 1) {            // = 1440
//		std::cerr << "Chuỗi chưa đủ 1440 phần tử\n";
//		return {};
//	}
//	return std::vector<T>(vec.begin() + StartIndex1,   // = ED_min
//		vec.begin() + EndIndex1 + 1);
//}

int main ()
{ 

// Read data from file
string Input_path = "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\UC_ED_CPLEX_TEST\\Data\\CSV Data";
string Output_path = "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\UC_ED_CPLEX_TEST\\ED\\Otput Data";

string BESSpath = Input_path + "\\BESS.csv";
string Maintenancepath = Input_path + "\\Maintenance.csv";
string DGpath = Input_path + "\\DG.csv";
string Gridpath = Input_path + "\\Grid.csv";
string Loadpath = Input_path + "\\Load.csv";
string REpath = Input_path + "\\RE Gen.csv";

CSVResult BESSdata = readCSV(BESSpath);
CSVResult MaintenanceData = readCSV(Maintenancepath);
CSVResult DGdata = readCSV(DGpath);
CSVResult Griddata = readCSV(Gridpath);
CSVResult Loaddata = readCSV(Loadpath);
CSVResult REdata = readCSV(REpath);

int NumBESS = BESSdata.data.size(); // Number of BESS
int NumDG = DGdata.data.size(); // Number of DG
int StepNum = Griddata.data.size(); // Number of Grid
double StepReSol = 1.0 / 60;
int LoadCoeff = 1; // Load coefficient
int RECoeff = 1; // RE coefficient
int GridBuyCoeff = 1; // Grid coefficient

// ---------------------------------Maintenance-----------------------
vector<vector<int>> DGMaintenance(NumDG, vector<int>(MaintenanceData.data.size()));
//vector<vector<int>> BattMaintenance(NumBESS, vector<int>(MaintenanceData.data.size()));
vector<int> BattMaintance;
vector<int> REMaintenancePV;
vector<int> REMaintenanceWind;
vector<int> LoadMaintenance;
vector<int> GridMaintenance;

for (size_t j = 0; j < MaintenanceData.data.size(); ++j) {
	for (size_t i = 0; i < NumDG; ++i) {
		DGMaintenance[i][j] = static_cast<int>(MaintenanceData.data[j][i]);
	}
	//for (size_t i = 0; i < NumBESS; ++i) {
	//	BattMaintenance[i][j] = static_cast<int>(MaintenanceData.data[j][i + NumDG]);
	//}

	BattMaintance.push_back(static_cast<int>(MaintenanceData.data[j][NumDG]));
	LoadMaintenance.push_back(static_cast<int>(MaintenanceData.data[j][NumDG + NumBESS]));
	REMaintenancePV.push_back(static_cast<int>(MaintenanceData.data[j][NumDG + NumBESS + 1]));
	REMaintenanceWind.push_back(static_cast<int>(MaintenanceData.data[j][NumDG + NumBESS + 2]));
	GridMaintenance.push_back(static_cast<int>(MaintenanceData.data[j][NumDG + NumBESS + 4]));
	// +3=SC1;   +4=SC2;   +5=SC3; 
}

//--------------------------------------- DG Data-----------------------------------------
vector<double> P_DGMin;
vector<double> P_DGMax;
vector<double> P_DGRate;
vector<double> F0;
vector<double> F1;
vector<double> F2;
vector<double> FuelPrices;
vector<double> StartUp_Cost;
vector<double> ShutDown_Cost;
vector<double> OMCost;
vector<double> REPCost;
vector<double> LeftTime;
vector<double> UpTimeMin;
vector<double> DownTimeMin;
vector<double> TimeInit;
vector<int> CommittedInitial;
for (size_t i = 0; i < DGdata.data.size(); ++i) {
	P_DGRate.push_back(DGdata.data[i][0]);
	P_DGMin.push_back(DGdata.data[i][1]);
	P_DGMax.push_back(DGdata.data[i][2]);
	F0.push_back(DGdata.data[i][3]);
	F1.push_back(DGdata.data[i][4]);
	F2.push_back(DGdata.data[i][5]);
	FuelPrices.push_back(DGdata.data[i][6]);
	StartUp_Cost.push_back(DGdata.data[i][7]);
	ShutDown_Cost.push_back(DGdata.data[i][8]);
	OMCost.push_back(DGdata.data[i][9]);
	REPCost.push_back(DGdata.data[i][10]);
	LeftTime.push_back(DGdata.data[i][11]);
	UpTimeMin.push_back(DGdata.data[i][12]);
	DownTimeMin.push_back(DGdata.data[i][13]);
	TimeInit.push_back(DGdata.data[i][14]);
	CommittedInitial.push_back(DGdata.data[i][15]);
}

// -----------------------------------------BESS data-----------------------------------
vector<double> PCS_Rate;
vector<double> BattCapa;
vector<double> RTeff;
vector<double> PCSeff;
vector<double> SocMin;
vector<double> SoCMax;
vector<double> SoCRev;
vector<double> BattSingleLine;
vector<double> NumSingleLine;
vector<double> BESS_Life_time;
vector<double> BESS_rep;
vector<double> SoC_ini;
for (size_t i = 0; i < BESSdata.data.size(); ++i)
{
	PCS_Rate.push_back(BESSdata.data[i][0]);         // PCSRated
	BattCapa.push_back(BESSdata.data[i][1]);         // BattCapacity
	RTeff.push_back(BESSdata.data[i][2]);            // RoundTripEfficiency
	PCSeff.push_back(BESSdata.data[i][3]);           // PCSEfficiency
	SocMin.push_back(BESSdata.data[i][4]);           // MinSoC
	SoCMax.push_back(BESSdata.data[i][5]);           // MaxSoC
	SoCRev.push_back(BESSdata.data[i][6]);           // SoCSpinning
	BattSingleLine.push_back(BESSdata.data[i][7]);   // BattSingleCapacity
	NumSingleLine.push_back(BESSdata.data[i][8]);    // NumSingleBatt
	BESS_Life_time.push_back(BESSdata.data[i][9]);   // LifetimeSingleBatt
	BESS_rep.push_back(BESSdata.data[i][10]);        // BESSRep
	SoC_ini.push_back(BESSdata.data[i][11]);         // SOCInitial
}

// --------------------------------------Grid data-----------------------------------
vector<double> GridBuyCost;
vector<double> GridSellCost;
vector<double> P_GridBuyMax;
vector<double> P_GridSellMax;
vector<double> P_GridBuyMin;
vector<double> P_GridSellMin;

for (size_t i = 0; i < Griddata.data.size(); i++)
{
	GridBuyCost.push_back(Griddata.data[i][4] * GridBuyCoeff);         // GridBuyCost
	GridSellCost.push_back(Griddata.data[i][5]);        // GridSellCost	
	P_GridBuyMin.push_back(Griddata.data[i][6] * GridMaintenance[i]);         // P_GridBuyMin
	P_GridBuyMax.push_back(Griddata.data[i][7] * GridMaintenance[i]);         // P_GridBuyMax
	P_GridSellMin.push_back(Griddata.data[i][8] * GridMaintenance[i]);        // P_GridSellMin
	P_GridSellMax.push_back(Griddata.data[i][9] * GridMaintenance[i]);        // P_GridSellMax

}

//--------------------------------------Load data-----------------------------------
vector<double> P_Load1;
vector<double> P_Load2;
vector<double> P_Load3;
vector<double> LoadShedingCost1;
vector<double> LoadShedingCost2;
vector<double> LoadShedingCost3;
vector<double> MaxLoadSheding;
vector<double> SR_required;
for (size_t i = 0; i < Loaddata.data.size(); ++i)
{
	P_Load1.push_back(Loaddata.data[i][7]); // P_Load1
	P_Load2.push_back(Loaddata.data[i][8]); // P_Load2
	P_Load3.push_back(Loaddata.data[i][9]); // P_Load3
	LoadShedingCost1.push_back(Loaddata.data[i][10]); // LoadShedingCost
	LoadShedingCost2.push_back(Loaddata.data[i][11]); // LoadShedingCost
	LoadShedingCost3.push_back(Loaddata.data[i][12]); // LoadShedingCost
	MaxLoadSheding.push_back(Loaddata.data[i][4]); // MaxLoadSheding
	SR_required.push_back(Loaddata.data[i][6]); // SR_required
}

//--------------------------------------RE data-----------------------------------
vector<double> P_RE_PV_ava;
vector<double> P_RE_Wind_ava;
for (size_t i = 0; i < REdata.data.size(); ++i)
{
	P_RE_PV_ava.push_back(REdata.data[i][4] * REMaintenancePV[i]); // P_RE_PV
	P_RE_Wind_ava.push_back(REdata.data[i][3] * REMaintenanceWind[i]); // P_RE_Wind
}
// ---------------------------------UC DATA-----------------------------------

string UCpath = "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\UC_ED_CPLEX_TEST\\Output\\Output_Microgrid_UC.csv";


CSVResult UCdata = readCSV(UCpath);

// Grid data UC
vector<double> P_GridBuy_UC;
vector<double> P_GridSell_UC;

// DG data UC
vector<double> U_DG1_UC;
vector<double> W_DG1_UC;
vector<double> V_DG1_UC;

vector<double> U_DG2_UC;
vector<double> W_DG2_UC;
vector<double> V_DG2_UC;

vector<double> U_DG3_UC;
vector<double> W_DG3_UC;
vector<double> V_DG3_UC;

vector<double> U_DG4_UC;
vector<double> W_DG4_UC;
vector<double> V_DG4_UC;

vector<double> P_DG1_UC;
vector<double> P_DG2_UC;
vector<double> P_DG3_UC;
vector<double> P_DG4_UC;

// BESS data UC

vector<double> SoC_BESS_UC;


for (size_t i = 0; i < UCdata.data.size(); ++i)
{
	// GRID data UC
	P_GridBuy_UC.push_back(UCdata.data[i][3]); // P_GridBuy
	P_GridSell_UC.push_back(UCdata.data[i][4]); // P_GridSell

	// DG data UC

	U_DG1_UC.push_back(UCdata.data[i][16]); // U_DG1
	U_DG2_UC.push_back(UCdata.data[i][17]); // U_DG2
	U_DG3_UC.push_back(UCdata.data[i][18]); // U_DG3
	U_DG4_UC.push_back(UCdata.data[i][19]); // U_DG4

	
	V_DG1_UC.push_back(UCdata.data[i][20]); // V_DG1
	V_DG2_UC.push_back(UCdata.data[i][21]); // V_DG2
	V_DG3_UC.push_back(UCdata.data[i][22]); // V_DG3
	V_DG4_UC.push_back(UCdata.data[i][23]); // V_DG4

	W_DG1_UC.push_back(UCdata.data[i][24]); // W_DG1
	W_DG2_UC.push_back(UCdata.data[i][25]); // W_DG2
	W_DG3_UC.push_back(UCdata.data[i][26]); // W_DG3
	W_DG4_UC.push_back(UCdata.data[i][27]); // W_DG4

	P_DG1_UC.push_back(UCdata.data[i][12]); // P_DG1
	P_DG2_UC.push_back(UCdata.data[i][13]); // P_DG2
	P_DG3_UC.push_back(UCdata.data[i][14]); // P_DG3
	P_DG4_UC.push_back(UCdata.data[i][15]); // P_DG4

	// BESS data UC
	SoC_BESS_UC.push_back(UCdata.data[i][30]); // SoC_BESS
}

// ED path
//string ED_DATA_path = "C:\\Users\\Ms.Ngoc\\OneDrive - ATS JSC\\Desktop\\UC_ED_CPLEX_TEST\\ED\\ED Data";

// Repeat 15 minutes to 1 minute Function Definition

const int ED_min = 840;       // <-- AT minute 362 of day
const int rep15to1 = 15;        // 15' → 15×1'
const int StartIndex1 = ED_min;    // 0-based
const int EndIndex1 = 1440 - 1;  // 23:59’  → 1439
const int StepNum1m = EndIndex1 - StartIndex1 + 1;
StepNum = StepNum1m; // 1440 - 362 = 1078

auto repeatStatus = [&](const std::vector<double>& in15)
	{
		std::vector<double> out;
		out.reserve(in15.size() * rep15to1);
		for (double v : in15)
			out.insert(out.end(), rep15to1, v);       // 15 lần v
		return out;
	};

auto repeatTrans = [&](const std::vector<double>& in15)
	{
		std::vector<double> out;
		out.reserve(in15.size() * rep15to1);
		for (double v : in15) {
			out.push_back(v);                         // phút đầu block
			out.insert(out.end(), rep15to1 - 1, 0.0); // 14 phút còn lại = 0
		}
		return out;
	};

/* ──────────────────────────────────────────────────────────────────
 * 4. CẮT PHẦN TỪ StartIndex ➜ EndIndex
 *    hàm slice() gọn gàng để tái sử dụng
 * ───────────────────────────────────────────────────────────────── */
auto slice = [&](std::vector<double>& vec) {
	if (vec.size() < EndIndex1 + 1) {
		std::cerr << "Chuỗi không đủ 1440 phần tử\n";
		return std::vector<double>();
	}
	return std::vector<double>(vec.begin() + StartIndex1,
		vec.begin() + EndIndex1 + 1);
	};


// ---------------------------------MEASUREMENT DATA-----------------------------------
string MEASUR_DATA_PATH = "C:\\Users\\Ms. Ngoc\\OneDrive - ATS JSC\\Desktop\\UC_ED_CPLEX_TEST\\ED\\ED Data\\RE Gen.csv";

std::vector<double> REMaintenancePV15(REMaintenancePV.begin(), REMaintenancePV.end());
std::vector<double> REMaintenanceWind15(REMaintenanceWind.begin(), REMaintenanceWind.end());

//std::vector<double> BattMaintenance15(BattMaintenance.begin(), BattMaintenance.end());

std::vector<double> REMaintenancePV1 = repeatStatus(REMaintenancePV15);
std::vector<double> REMaintenanceWind1 = repeatStatus(REMaintenanceWind15);

//std::vector<double> BESSMaintenance1 = repeatStatus(BattMaintenance);


CSVResult MEASUR_DATA = readCSV(MEASUR_DATA_PATH);

vector<double> P_Load_Meas1;
vector<double> P_Load_Meas2;
vector<double> P_Load_Meas3;
vector<double> P_RE_PV_Meas;
vector<double> P_RE_Wind_Meas;

for (int t = 0; t < 1440; ++t)
{
	P_Load_Meas1.push_back(MEASUR_DATA.data[t][3]); // Load_Meas
	P_Load_Meas2.push_back(MEASUR_DATA.data[t][4]); // Load_Meas2
	P_Load_Meas3.push_back(MEASUR_DATA.data[t][5]); // Load_Meas3
	P_RE_PV_Meas.push_back(MEASUR_DATA.data[t][1] * REMaintenancePV1[t]); // PV_Meas
	P_RE_Wind_Meas.push_back(MEASUR_DATA.data[t][0] * REMaintenancePV1[t]); // WTG_Meas
}

/* ──────────────────────────────────────────────────────────────────
 * 3. 15 min --> 1 min
 * ───────────────────────────────────────────────────────────────── */
// GRID 15 min
std::vector<double> GridBuy15(P_GridBuy_UC.begin(), P_GridBuy_UC.end());
std::vector<double> GridSell15(P_GridSell_UC.begin(), P_GridSell_UC.end());
std::vector<double> GridBuyCost15(GridBuyCost.begin(), GridBuyCost.end()); // 96
std::vector<double> GridSellCost15(GridSellCost.begin(), GridSellCost.end()); // 96
std::vector<double> P_GRID_Buy_min15(P_GridBuyMin.begin(), P_GridBuyMin.end());
std::vector<double> P_GRID_Buy_max15(P_GridBuyMax.begin(), P_GridBuyMax.end()); // 96
std::vector<double> P_GRID_Sell_min15(P_GridSellMin.begin(), P_GridSellMin.end());
std::vector<double> P_GRID_Sell_max15(P_GridSellMax.begin(), P_GridSellMax.end()); // 96
std::vector<double> GridMaintainance_15(GridMaintenance.begin(), GridMaintenance.end()); // 96
std::vector<double> BESSMaintenance_15(BattMaintance.begin(), BattMaintance.end()); // 96

/* DG 1-min */
std::vector<double> DG1_W15(W_DG1_UC.begin(), W_DG1_UC.end());   // shut-down
std::vector<double> DG1_V15(V_DG1_UC.begin(), V_DG1_UC.end());   // start-up
std::vector<double> DG1_U15(U_DG1_UC.begin(), U_DG1_UC.end());   // status

std::vector<double> DG2_W15(W_DG2_UC.begin(), W_DG2_UC.end());   // shut-down
std::vector<double> DG2_V15(V_DG2_UC.begin(), V_DG2_UC.end());   // start-up
std::vector<double> DG2_U15(U_DG2_UC.begin(), U_DG2_UC.end());   // status

std::vector<double> DG3_W15(W_DG3_UC.begin(), W_DG3_UC.end());   // shut-down
std::vector<double> DG3_V15(V_DG3_UC.begin(), V_DG3_UC.end());   // start-up
std::vector<double> DG3_U15(U_DG3_UC.begin(), U_DG3_UC.end());   // status

std::vector<double> DG4_W15(W_DG4_UC.begin(), W_DG4_UC.end());   // shut-down
std::vector<double> DG4_V15(V_DG4_UC.begin(), V_DG4_UC.end());   // start-up
std::vector<double> DG4_U15(U_DG4_UC.begin(), U_DG4_UC.end());   // status


/* Grid 1-min */
std::vector<double> GridBuy1 = repeatStatus(GridBuy15);
std::vector<double> GridSell1 = repeatStatus(GridSell15);
std::vector<double> GridBuyCost1 = repeatStatus(GridBuyCost15); // 1440
std::vector<double> GridSellCost1 = repeatStatus(GridSellCost15); // 1440
std::vector<double> P_GridBuyMin1 = repeatStatus(P_GRID_Buy_min15); // 1440
std::vector<double> P_GridBuyMax1 = repeatStatus(P_GRID_Buy_max15); // 1440
std::vector<double> P_GridSellMin1 = repeatStatus(P_GRID_Sell_min15); // 1440
std::vector<double> P_GridSellMax1 = repeatStatus(P_GRID_Sell_max15); // 1440
std::vector<double> GridMaintainance1 = repeatStatus(GridMaintainance_15); // 1440
std::vector<double> BESSMaintenance1 = repeatStatus(BESSMaintenance_15); // 1440

std::vector<double> DG1_W1 = repeatTrans(DG1_W15);
std::vector<double> DG1_V1 = repeatTrans(DG1_V15);
std::vector<double> DG1_U1 = repeatStatus(DG1_U15);

std::vector<double> DG2_W1 = repeatTrans(DG2_W15);
std::vector<double> DG2_V1 = repeatTrans(DG2_V15);
std::vector<double> DG2_U1 = repeatStatus(DG2_U15);

std::vector<double> DG3_W1 = repeatTrans(DG3_W15);
std::vector<double> DG3_V1 = repeatTrans(DG3_V15);
std::vector<double> DG3_U1 = repeatStatus(DG3_U15);

std::vector<double> DG4_W1 = repeatTrans(DG4_W15);
std::vector<double> DG4_V1 = repeatTrans(DG4_V15);
std::vector<double> DG4_U1 = repeatStatus(DG4_U15);



/* BESS 1 min */
std::vector<double> SoC15(SoC_BESS_UC.begin(), SoC_BESS_UC.end());
std::vector<double> SoC1 = repeatStatus(SoC15);

/* Load forcast*/
std::vector<double> Load15_measure1(P_Load_Meas1.begin(), P_Load_Meas1.end());
std::vector<double> Load15_measure2(P_Load_Meas2.begin(), P_Load_Meas2.end());
std::vector<double> Load15_measure3(P_Load_Meas3.begin(), P_Load_Meas3.end());

std::vector<double> Load1Forcast_fc(P_Load1.begin(), P_Load1.end());      // 96
std::vector<double> Load2Forcast_fc(P_Load2.begin(), P_Load2.end());     // 96
std::vector<double> Load3Forcast_fc(P_Load3.begin(), P_Load3.end());     // 96
std::vector<double> MaxLoadShedidng15_fc(MaxLoadSheding.begin(), MaxLoadSheding.end());   // 96
std::vector<double> LoadShedingCost1_fc(LoadShedingCost1.begin(), LoadShedingCost1.end());   // 96
std::vector<double> LoadShedingCost2_fc(LoadShedingCost2.begin(), LoadShedingCost2.end());   // 96
std::vector<double> LoadShedingCost3_fc(LoadShedingCost3.begin(), LoadShedingCost3.end());   // 96
std::vector<double> PV15_fc(P_RE_PV_ava.begin(), P_RE_PV_ava.end());   // 96
std::vector<double> WTG15_fc(P_RE_Wind_ava.begin(), P_RE_Wind_ava.end());

/* 6.1  Lặp lại mỗi giá trị 15 lần  (15' ➜ 1') */

std::vector<double> PV1_fc = repeatStatus(PV15_fc);      // 1440
std::vector<double> WTG1_fc = repeatStatus(WTG15_fc);     // 1440
std::vector<double> Load1_measur_1 = Load15_measure1;   // 1440 
std::vector<double> Load2_measur_1 = Load15_measure2;   // 1440
std::vector<double> Load3_measur_1 = Load15_measure3;   // 1440
std::vector<double> Load1m_forcast = repeatStatus(Load1Forcast_fc);   // 1440
std::vector<double> Load2m_forcast = repeatStatus(Load2Forcast_fc);   // 1440
std::vector<double> Load3m_forcast      = repeatStatus(Load3Forcast_fc);   // 1440
std::vector<double> MaxLoadShedding1m   = repeatStatus(MaxLoadShedidng15_fc);   // 1440
std::vector<double> LoadShedingCost1_1m = repeatStatus(LoadShedingCost1_fc);   // 1440
std::vector<double> LoadShedingCost2_1m = repeatStatus(LoadShedingCost2_fc);   // 1440
std::vector<double> LoadShedingCost3_1m = repeatStatus(LoadShedingCost3_fc);   // 1440


GridBuy1  = slice(GridBuy1); // Power Grid Buy
GridSell1 = slice(GridSell1);// Power Grid Sell 
GridBuyCost1 = slice(GridBuyCost1); // Grid Buy Cost
GridSellCost1 = slice(GridSellCost1); // Grid Sell Cost
P_GridBuyMin1 = slice(P_GridBuyMin1); // P_GridBuyMin
P_GridBuyMax1 = slice(P_GridBuyMax1); // P_GridBuyMax
P_GridSellMin1 = slice(P_GridSellMin1); // P_GridSellMin
P_GridSellMax1 = slice(P_GridSellMax1); // P_GridSellMax
GridMaintainance1 = slice(GridMaintainance1); // Grid Maintainance
BESSMaintenance1 = slice(BESSMaintenance1); // BESS Maintenance

DG1_W1    = slice(DG1_W1);
DG1_V1    = slice(DG1_V1);
DG1_U1    = slice(DG1_U1);

DG2_W1 = slice(DG2_W1);
DG2_V1 = slice(DG2_V1);
DG2_U1 = slice(DG2_U1);

DG3_W1 = slice(DG3_W1);
DG3_V1 = slice(DG3_V1);
DG3_U1 = slice(DG3_U1);

DG4_W1 = slice(DG4_W1);
DG4_V1 = slice(DG4_V1);
DG4_U1 = slice(DG4_U1);

SoC1      = slice(SoC1);
P_RE_PV_Meas      = slice(P_RE_PV_Meas);
P_RE_Wind_Meas    = slice(P_RE_Wind_Meas);
MaxLoadShedding1m = slice(MaxLoadShedding1m); // MaxLoadSheding
LoadShedingCost1_1m = slice(LoadShedingCost1_1m); // LoadShedingCost1
LoadShedingCost2_1m = slice(LoadShedingCost2_1m); // LoadShedingCost2
LoadShedingCost3_1m = slice(LoadShedingCost3_1m); // LoadShedingCost3

PV1_fc	  = slice(PV1_fc);          //
WTG1_fc	  = slice(WTG1_fc);
Load1_measur_1  = slice(Load1_measur_1);      //
Load2_measur_1 = slice(Load2_measur_1);      //
Load3_measur_1 = slice(Load3_measur_1);      //
Load1m_forcast = slice(Load1m_forcast);      //
Load2m_forcast = slice(Load2m_forcast);      //
Load3m_forcast = slice(Load3m_forcast);      //

std::vector<double> PV_used          = PV1_fc;
std::vector<double> WTG_used	     = WTG1_fc;

/* ---- 5.  Thay giá trị dự báo phút ED bằng số đo thực tế ---- */
if (!Load1m_forcast.empty() && !Load1_measur_1.empty())
Load1m_forcast[0] = Load1_measur_1[0];   // phút đầu tiên (ED)

if (!Load2m_forcast.empty() && !Load2_measur_1.empty())
Load2m_forcast[0] = Load2_measur_1[0];

if (!Load3m_forcast.empty() && !Load3_measur_1.empty())
Load3m_forcast[0] = Load3_measur_1[0];

if (!PV1_fc.empty() && !P_RE_PV_Meas.empty())
PV1_fc[0] = P_RE_PV_Meas[0];   // phút đầu tiên (ED)

if (!WTG1_fc.empty() && !P_RE_Wind_Meas.empty())
WTG1_fc[0] = P_RE_Wind_Meas[0];   // phút đầu tiên (ED)

// -----------------------------Piecewise linear function for fuel cost-----------------------------
std::vector<double> FuelFixed(NumDG);   // phần cố định ở Pmin
std::vector<double> FuelSlope(NumDG);   // hệ số tuyến tính cho P>Pmin

for (int g = 0; g < NumDG; ++g)
{
	double Pmin = P_DGMin[g];
	double Pmax = P_DGMax[g];

	/* 1. giá trị hàm tại Pmin & Pmax (chưa nhân giá diesel) */
	auto quad = [&](double P) { return F0[g] + F1[g] * P + F2[g] * P * P; };

	double Fmin = quad(Pmin);
	double Fmax = quad(Pmax);

	/* 2. phần cố định ở Pmin (đã nhân giá diesel & ∆t)        */
	FuelFixed[g] = Fmin * FuelPrices[g];          // $ ⋅ h

	/* 3. slope cho duy nhất 1 segment                        */
	FuelSlope[g] = (Fmax - Fmin) / (Pmax - Pmin) * FuelPrices[g];
}

//-------------------------------------- CPLEX model define -----------------------------------
IloEnv env;
IloModel model(env);

// --------------------------------------Grid Variables------------------------------------
IloNumVarArray P_GridBuy(env);
IloNumVarArray P_GridSell(env);
IloNumVarArray P_GridBuyExtra(env);
IloNumVarArray P_GridSellExtra(env);
for (size_t i = 0; i < StepNum1m; i++)
{
	P_GridBuy.add(IloNumVar(env, GridBuy1[i], GridBuy1[i]));
	P_GridSell.add(IloNumVar(env, GridSell1[i], GridSell1[i]));
	P_GridBuyExtra.add(IloNumVar(env, P_GridBuyMin1[i], P_GridBuyMax1[i])); // P_GridBuyExtra
	P_GridSellExtra.add(IloNumVar(env, P_GridSellMin1[i], P_GridSellMax1[i])); // P_GridSellExtra
}
// Grid Cost
IloExpr totalGridBuyCost(env);
IloExpr totalGridSellCost(env);
for (int j = 0; j < StepNum1m; j++) {
	totalGridBuyCost += P_GridBuy[j] * GridBuyCost1[j] * StepReSol;
	totalGridSellCost += -P_GridSell[j] * GridSellCost1[j] * StepReSol;
	totalGridBuyCost += P_GridBuyExtra[j] * GridBuyCost1[j] * StepReSol * 5;
	totalGridSellCost += P_GridSellExtra[j] * GridSellCost1[j] * StepReSol * 1e-6 ;
}
//
// ---------------------------------DG Variables-----------------------------------
std::vector<std::vector<int>> U_Fixed(NumDG, std::vector<int>(StepNum1m));

U_Fixed[0].assign(DG1_U1.begin(), DG1_U1.end());
U_Fixed[1].assign(DG2_U1.begin(), DG2_U1.end());
U_Fixed[2].assign(DG3_U1.begin(), DG3_U1.end());
U_Fixed[3].assign(DG4_U1.begin(), DG4_U1.end());
IloArray<IloBoolVarArray> Committed(env, NumDG); // U
IloArray<IloNumVarArray> PowerDG(env, NumDG); // P_DG

for (int g = 0; g < NumDG; ++g)
{
	Committed[g] = IloBoolVarArray(env, StepNum1m);

	for (int t = 0; t < StepNum1m; ++t)
	{
		// tạo BoolVar rồi khóa LB = UB = giá trị UC
		Committed[g][t] = IloBoolVar(env);
		const int fixVal = U_Fixed[g][t];               // 0 hoặc 1
		Committed[g][t].setBounds(fixVal, fixVal);      // “freeze”
	}
}

for (size_t i = 0; i < NumDG; i++)
{
	PowerDG[i] = IloNumVarArray(env, StepNum1m, 0, P_DGMax[i]);
}
// Min Max Power DG constraint Pmin[i][t]*U[i][t] <= P_DG[i][t] <= Pmax[i][t]*U[i][t]
for (size_t i = 0; i < NumDG; i++)
{
	for (size_t t = 0; t < StepNum1m; t++)
	{
		IloExpr PminDGConstriant(env);
		PminDGConstriant += P_DGMin[i] * Committed[i][t];
		model.add(PminDGConstriant <= PowerDG[i][t]);
		PminDGConstriant.end();

		IloExpr PmaxDGConstriant(env);
		PmaxDGConstriant += P_DGMax[i] * Committed[i][t];
		model.add(PmaxDGConstriant >= PowerDG[i][t]);
		PmaxDGConstriant.end();
	}
}

// -------------DG Cost Definetion--------------------

// Fuel Cost
IloExpr totalFuelCost(env);

for (int g = 0; g < NumDG; ++g)
{
	const double cFix = FuelFixed[g] * StepReSol;   // $$ cố định
	const double cSlope = FuelSlope[g] * StepReSol;   // $$/kW

	for (int t = 0; t < StepNum1m; ++t)
	{
		/* --- thành phần cố định: cFix × U(g,t) --- */
		totalFuelCost += cFix * Committed[g][t];

		/* --- thành phần tuyến tính: cSlope × (P – Pmin·U) --- */
		IloExpr incP(env);
		incP += PowerDG[g][t];                 // P(g,t)
		incP += -P_DGMin[g] * Committed[g][t]; // − Pmin·U(g,t)

		totalFuelCost += cSlope * incP;
		incP.end();
	}
}
/* ------------------------------------------------------------------
*  DG-priority  (1 = ưu tiên cao, 2 = thấp hơn, …)
* ------------------------------------------------------------------*/
const std::vector<double> DGPriority = { 2, 1, 2, 1 };   // nDG phần tử
const double epsPriority = 1e-5;                      // giống Config.DG.eps_Priority

/* penalty(g) = priority(g) · epsPriority  */
std::vector<double> priorityPenalty(NumDG);
for (int g = 0; g < NumDG; ++g)
	priorityPenalty[g] = DGPriority[g] * epsPriority;

for (int g = 0; g < NumDG; ++g)
	for (int t = 0; t < StepNum; ++t)
	{
		/* mở máy */
		totalFuelCost += priorityPenalty[g] * Committed[g][t];
	}
for (int g = 0; g < NumDG; ++g)
	for (int t = 0; t < StepNum; ++t)
		totalFuelCost += priorityPenalty[g] * PowerDG[g][t];        // kW·ε·P(g)
// ---------------------------------RE Variables-----------------------------------
IloNumVarArray P_RE_PV(env); // P_RE_PV
IloNumVarArray P_RE_Wind(env); // P_RE_Wind
for (int j = 0; j < StepNum1m; j++)
{
	P_RE_PV.add(IloNumVar(env, 0, PV1_fc[j]));
	P_RE_Wind.add(IloNumVar(env, 0, WTG1_fc[j]));
}

IloNumVarArray P_RE_PV_Cur(env); // P_RE_PV_Cur
IloNumVarArray P_RE_Wind_Cur(env); // P_RE_Wind_Cur
for (int j = 0; j < StepNum1m; j++)
{
	P_RE_PV_Cur.add(IloNumVar(env, 0, PV1_fc[j]));
	P_RE_Wind_Cur.add(IloNumVar(env, 0, WTG1_fc[j]));
}

for (int j = 0; j < StepNum1m; ++j)
{
	// PV:  P_RE_PV[j]  +  P_RE_PV_Cur[j]  =  P_RE_PV_ava[j]
	model.add(P_RE_PV[j] + P_RE_PV_Cur[j] == PV1_fc[j]);
	// WTG: P_RE_Wind[j] + P_RE_Wind_Cur[j] = P_RE_Wind_ava[j]
	model.add(P_RE_Wind[j] + P_RE_Wind_Cur[j] == WTG1_fc[j]);
}

//------------- RE Cost Definetion -------------------

IloExpr Pv_Cur_cost(env);
IloExpr Wind_Cur_cost(env);
for (int j = 0; j < StepNum; j++)
{
	Pv_Cur_cost += P_RE_PV_Cur[j] * 0.1 * StepReSol;
	Wind_Cur_cost += P_RE_Wind_Cur[j] * 0.3 * StepReSol;
}
//
// --------------------------------- BESS Variables -----------------------------------
IloArray<IloNumVarArray> SoC(env, NumBESS); // SoC_BESS
IloArray<IloNumVarArray> P_BESS_dch(env, NumBESS); // P_BESS_dch
IloArray<IloNumVarArray> P_BESS_ch(env, NumBESS); // P_BESS_ch
IloArray<IloBoolVarArray> U_BESS_dch(env, NumBESS);   // 1 → đang Dch
IloArray<IloBoolVarArray> U_BESS_ch(env, NumBESS);   // 1 → đang Ch
IloArray<IloBoolVarArray> O_BESS(env, NumBESS); // SR Decision ) -> no SR, 1-> SR
IloArray<IloNumVarArray> P_SR(env, NumBESS); // P_SR (kW)
// SoC , P_dch , P_ch Limit 
for (int b = 0; b < NumBESS; ++b)
{
	/* 1. Biến liên tục */
	SoC[b] = IloNumVarArray(env, StepNum1m, SocMin[b], SoCMax[b]);   // %SOC
	P_BESS_dch[b] = IloNumVarArray(env, StepNum1m, 0, PCS_Rate[b]); // kW xả
	P_BESS_ch[b] = IloNumVarArray(env, StepNum1m, 0, PCS_Rate[b]); // kW nạp
	P_SR[b] = IloNumVarArray(env, StepNum1m, 0, PCS_Rate[b]); // kW SR

	/* 2. Biến nhị phân (logic) */
	U_BESS_dch[b] = IloBoolVarArray(env, StepNum1m);   // 1 → đang xả
	U_BESS_ch[b] = IloBoolVarArray(env, StepNum1m);   // 1 → đang nạp
	O_BESS[b] = IloBoolVarArray(env, StepNum1m);   // 1 → thỏa SoC ≥ SoC_rev (được tính SR)
}
/* --------------------------------------------------------
RAMP-SMOOTHING VARIABLES  (áp dụng cho mọi BESS)
-------------------------------------------------------- */
const double M_ramp = 10000; // “big-M”

/* r_plus , r_minus       : phần dương / âm của ΔP          *
 * y_sign                 : nhị phân, 1 nếu ΔP ≥ 0          *
 * Ramp_dch , Ramp_ch     : |ΔP| = r_plus + r_minus (đưa vào cost) */
IloArray<IloNumVarArray>  r_plus_dch(env, NumBESS);
IloArray<IloNumVarArray>  r_minus_dch(env, NumBESS);
IloArray<IloBoolVarArray> y_dch(env, NumBESS);   // sign của ΔP dch

IloArray<IloNumVarArray>  r_plus_ch(env, NumBESS);
IloArray<IloNumVarArray>  r_minus_ch(env, NumBESS);
IloArray<IloBoolVarArray> y_ch(env, NumBESS);   // sign của ΔP ch

IloArray<IloNumVarArray>  Ramp_dch(env, NumBESS);   // |ΔP_dch|
IloArray<IloNumVarArray>  Ramp_ch(env, NumBESS);   // |ΔP_ch |

for (int b = 0; b < NumBESS; ++b)
{
	r_plus_dch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
	r_minus_dch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
	y_dch[b] = IloBoolVarArray(env, StepNum1m);

	r_plus_ch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
	r_minus_ch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
	y_ch[b] = IloBoolVarArray(env, StepNum1m);

	Ramp_dch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
	Ramp_ch[b] = IloNumVarArray(env, StepNum1m, 0, M_ramp);
}

// Ramp BESS miminiazation
for (int b = 0; b < NumBESS; ++b)
{
	for (int t = 1; t < StepNum1m; ++t)      //   t = 1 … T-1
	{
		/* ---------- 2.1  Phần discharge ---------- */
		/*  (a)  ΔP  =  P_dch(t) - P_dch(t-1) = r_plus - r_minus         */
		{
			IloExpr expr(env);
			expr += P_BESS_dch[b][t] - P_BESS_dch[b][t - 1];
			expr -= r_plus_dch[b][t];
			expr += r_minus_dch[b][t];
			model.add(expr == 0); expr.end();
		}
		/*  (b)  0 ≤ r_plus ≤ M·y   ;   0 ≤ r_minus ≤ M·(1-y)            */
		model.add(r_plus_dch[b][t] <= M_ramp * y_dch[b][t]);
		model.add(r_minus_dch[b][t] <= M_ramp * (1 - y_dch[b][t]));

		/*  (c)  Ramp_dch = r_plus + r_minus  (= |ΔP|)                   */
		{
			IloExpr expr(env);
			expr += Ramp_dch[b][t] - r_plus_dch[b][t] - r_minus_dch[b][t];
			model.add(expr == 0); expr.end();
		}

		/* ---------- 2.2  Phần charge ----------   (tương tự) */
		{
			IloExpr expr(env);
			expr += P_BESS_ch[b][t] - P_BESS_ch[b][t - 1];
			expr -= r_plus_ch[b][t];
			expr += r_minus_ch[b][t];
			model.add(expr == 0); expr.end();
		}
		model.add(r_plus_ch[b][t] <= M_ramp * y_ch[b][t]);
		model.add(r_minus_ch[b][t] <= M_ramp * (1 - y_ch[b][t]));

		{
			IloExpr expr(env);
			expr += Ramp_ch[b][t] - r_plus_ch[b][t] - r_minus_ch[b][t];
			model.add(expr == 0); expr.end();
		}
	}
}

// SoC Dynamic Constraints

std::vector<double> SoC_ED_Init(NumBESS, 0.0);
for (int b = 0; b < NumBESS; ++b)
{
	/* Nếu có nhiều BESS, hãy gán SoC1_b[0] tương ứng.
	   Ở ví dụ 1 BESS: SoC1[0] chính là %SoC tại phút ED_min */
	SoC_ED_Init[b] = SoC1[0];
}

for (int i = 0; i < NumBESS; i++)
{
	for (int j = 0; j < StepNum; j++)
	{
		if (j == 0)
		{
			if (j == 0)
			{
				IloNumExpr d = SoC[i][j] * BattCapa[i];
				IloNumExpr u1 = IloNumExpr(env, SoC_ED_Init[i] * BattCapa[i]);
				IloNumExpr u2 = P_BESS_ch[i][j] * PCSeff[i] * sqrt(RTeff[i]) * StepReSol;
				IloNumExpr u3 = -P_BESS_dch[i][j] * StepReSol / (PCSeff[i] * sqrt(RTeff[i]));
				model.add(d == (u1 + u2 + u3));
				d.end();
				u1.end();
				u2.end();
				u3.end();
			}
		}
		else
		{
			IloNumExpr d = SoC[i][j] * BattCapa[i];
			IloNumExpr dch = -P_BESS_dch[i][j] * StepReSol / (sqrt(RTeff[i]) * PCSeff[i]);
			IloNumExpr ch = P_BESS_ch[i][j] * StepReSol * (sqrt(RTeff[i]) * PCSeff[i]);
			IloNumExpr soc_1 = SoC[i][j - 1] * BattCapa[i];
			model.add(d == (dch + ch + soc_1));
			d.end();
			dch.end();
			ch.end();
			soc_1.end();
		}

	}
}

////---------------------------SoC Dynamic Constraints 2-------------------------
//std::vector<double> SoC_ED_Init(NumBESS);
//SoC_ED_Init[0] = SoC1.front();
//
//for (int b = 0; b < NumBESS; ++b)
//{
//	const double sqrtRT = std::sqrt(RTeff[b]);              // √ε_rt
//	const double eta_ch = sqrtRT * PCSeff[b];               // η_charge
//	const double eta_dch_inv = 1.0 / eta_ch;                // 1/η_discharge
//	const double k_ch = StepReSol * eta_ch / BattCapa[b];   //  Δt·η_ch / C
//	const double k_dch = StepReSol * eta_dch_inv / BattCapa[b];   //  Δt/(η_dch·C)
//
//	/* ========= 2.  SOC(0) = SOC_ini ========= */
//	model.add(SoC[b][0] == SoC_ED_Init[b]);
//
//	/* ========= 3.  SOC dynamics for t = 1 … T-1 ========= *
//	 *    SOC(t) − SOC(t-1)
//	 *  + k_dch · P_dch(t-1)
//	 *  − k_ch  · P_ch (t-1)  = 0                        */
//	for (int t = 1; t < StepNum1m; ++t)
//	{
//		IloExpr socDyn(env);
//		socDyn += SoC[b][t];							//  + SOC(t)
//		socDyn += -SoC[b][t - 1];						//  − SOC(t-1)
//		socDyn += k_dch * P_BESS_dch[b][t];			//  + k_dch·P_dch(t-1)
//		socDyn += -k_ch * P_BESS_ch[b][t];			//  − k_ch ·P_ch (t-1)
//
//		model.add(socDyn == 0);
//		socDyn.end();
//	}
//}

/* ---------- Pmax & logic Constraint ---------------- */
for (int i = 0; i < NumBESS; ++i)
{
	/* 2 hằng số α₁, α₂ của pin i */
	const double sqrtRT = std::sqrt(RTeff[i]);
	const double k1 = sqrtRT * PCSeff[i];						 // √ε_rt × ε_PCS
	const double k2 = 1.0 / k1;									 // 1 / (√ε_rt × ε_PCS)
	const double alpha1 = BattCapa[i] * k1 / StepReSol;          // (C × k1)/Δt
	const double alpha2 = BattCapa[i] * k2 / StepReSol;          // (C × k2)/Δt

	for (int t = 0; t < StepNum1m; ++t)
	{
		/* --- 1. Giới hạn bởi PCS_rated & công-tắc nhị phân --- */
		model.add(P_BESS_dch[i][t] <= BESSMaintenance1[t] * U_BESS_dch[i][t] * PCS_Rate[i]);
		model.add(P_BESS_ch[i][t] <= BESSMaintenance1[t] * U_BESS_ch[i][t] * PCS_Rate[i]);
		/* --- 2. P_BESSdch,max = min( … )  --------------------- *
			 P_dch - α1·(SoC - SoC_min) ≤ 0                       */
		{
			IloExpr e(env);
			e += P_BESS_dch[i][t];
			e += -alpha1 * (SoC[i][t] - SocMin[i]);
			model.add(e <= 0);
			e.end();
		}
		/* --- 3. P_BESSch,max = min( … )  ---------------------- *
			 P_ch + α2·SoC ≤ α2·SoC_max                           */
		{
			IloExpr e(env);
			e += P_BESS_ch[i][t] + alpha2 * SoC[i][t];
			model.add(e <= alpha2 * SoCMax[i]);
			e.end();
		}
		/* --- 4. Logic: không được vừa xả vừa nạp --------------- */
		{
			IloExpr logic(env);
			logic += U_BESS_dch[i][t] + U_BESS_ch[i][t];
			model.add(logic <= 1);              // =1  (nếu cho phép “idle” thì đổi thành ≤1)
			logic.end();
		}
	}
}

//------------------- O_BESS & P_SR -------------------
for (int i = 0; i < NumBESS; ++i) {
	O_BESS[i] = IloBoolVarArray(env, StepNum1m);
	P_SR[i] = IloNumVarArray(env, StepNum1m, 0, PCS_Rate[i]);
}

const double bigM = 1.0;         // ≥ (SoCmax-SoCmin)

for (int i = 0; i < NumBESS; ++i)
{
	const double sqrtRT = std::sqrt(RTeff[i]);
	const double k1 = sqrtRT * PCSeff[i];
	const double alpha1 = (BattCapa[i] * k1 / StepReSol)/15;

	for (int t = 0; t < StepNum1m; ++t)
	{
		const double gFlag = 1.0 - static_cast<double>(GridMaintainance1[t]);   // 0 on-grid, 1 island

		/* ---- (A) Định nghĩa O_BESS chỉ có hiệu lực khi island ---- */
		{
			IloExpr e1(env);               // gFlag·(SoC - SoCrev) ≤ M·O
			e1 += gFlag * (SoC[i][t] - SoCRev[i]) - bigM * O_BESS[i][t];
			model.add(e1 <= 0);  e1.end();

			IloExpr e2(env);               // gFlag·(SoCrev - SoC) ≤ M·(1-O)
			e2 += gFlag * (SoCRev[i] - SoC[i][t]) + bigM * O_BESS[i][t];
			model.add(e2 <= bigM); e2.end();
		}

		/* ---- (B) Định nghĩa P_SR (tắt khi on-grid) ---- */
		model.add(P_SR[i][t] <= gFlag * PCS_Rate[i] * O_BESS[i][t]);

		IloExpr e(env);                    // P_SR ≤ gFlag·α1(SoC-SoCmin)
		e += P_SR[i][t] - gFlag * alpha1 * (SoC[i][t] - SocMin[i]);
		model.add(e <= 0);  e.end();
	}
}

// ---------------BESS Cost Definiton -----------------------------
IloExpr totalBESSCost(env);

// dch cost
for (int i = 0; i < NumBESS; ++i)
{
	/* hệ số chi phí mòn pin của BESS i */
	const double C_battwear =
		BESS_rep[i] /
		(NumSingleLine[i] * BESS_Life_time[i] * sqrt(RTeff[i]));
	for (int t = 0; t < StepNum1m; ++t)
	{
		totalBESSCost +=
			C_battwear                           /* $/kWh chu kỳ  */
			* (P_BESS_dch[i][t] / PCSeff[i])     /* kW vào pin     */
			* StepReSol;                           /* quy đổi về $   */
	}
}

// SR define cost
const double SR_REVENUE = -1e-5;           //  âm ⇒ mô hình muốn P_SR lớn nhất
for (int i = 0; i < NumBESS; ++i)
{
	for (int t = 0; t < StepNum1m; ++t)
	{
		totalBESSCost +=
			SR_REVENUE          //  $/(kWh)
			* P_SR[i][t]          //  kW
			/**StepReSol*/
			;          //  => $
	}
}
// BESS use times cost
const double alpha_use = 1e-5;

for (int b = 0; b < NumBESS; ++b)
{
	for (int t = 0; t < StepNum1m; ++t)
	{
		/* U = 1 khi pin hoạt động */
		totalBESSCost += alpha_use * (U_BESS_dch[b][t] + U_BESS_ch[b][t]);
	}
}

// ------------- Load Sheding Variables -------------------
IloNumVarArray P_LoadShed1(env); // P_LoadShed1
IloNumVarArray P_LoadShed2(env); // P_LoadShed2
IloNumVarArray P_LoadShed3(env); // P_LoadShed3
for (int j = 0; j < StepNum1m; j++)
{
	P_LoadShed1.add(IloNumVar(env, 0, Load1m_forcast[j]));
	P_LoadShed2.add(IloNumVar(env, 0, Load2m_forcast[j]));
	P_LoadShed3.add(IloNumVar(env, 0, Load3m_forcast[j]));
}
for (int j = 0; j < StepNum1m; j++)
{
	model.add(P_LoadShed1[j] + P_LoadShed2[j] + P_LoadShed3[j] <= MaxLoadShedding1m[j]);
}

// ------------------- Load Sheding Cost Definetion -------------------
IloExpr totalLoadShedCost(env);
/*IloExpr totalLoadShedCost2(env);
IloExpr totalLoadShedCost3(env);*/
for (int j = 0; j < StepNum1m; j++)
{
	totalLoadShedCost += LoadShedingCost1_1m[j] * P_LoadShed1[j] * StepReSol;
	totalLoadShedCost += LoadShedingCost2_1m[j] * P_LoadShed2[j] * StepReSol;
	totalLoadShedCost += LoadShedingCost3_1m[j] * P_LoadShed3[j] * StepReSol;
}
//
// ------------------- Power Balance Constraints -------------------
for (int j = 0; j < StepNum; ++j)
{
	IloExpr balance(env);

	/* 1)  P_gridbuy(j) – P_gridsell(j) */
	balance += P_GridBuy[j];
	balance += P_GridBuyExtra[j]; // P_GridBuyExtra
	balance -= P_GridSell[j];
	balance -= P_GridSellExtra[j]; // P_GridSellExtra

	/* 2)  Σ_i  P_DG(i,j) */
	for (int i = 0; i < NumDG; ++i)
		balance += PowerDG[i][j];

	/* 3)  RE (PV + Wind) */
	balance += P_RE_PV[j];
	balance += P_RE_Wind[j];

	/* 4)  BESS  (+dch − ch) */
	for (int i = 0; i < NumBESS; ++i) {
		balance += P_BESS_dch[i][j];
		balance -= P_BESS_ch[i][j];
	}

	/* 5)  Nhu cầu ròng = (P_Load1+2+3) − (shed1+2+3)  */
	IloExpr netLoad(env);
	netLoad += (Load1m_forcast[j] + Load2m_forcast[j] + Load3m_forcast[j]);
	netLoad -= (P_LoadShed1[j] + P_LoadShed2[j] + P_LoadShed3[j]);

	/* 6)  Cân bằng công suất */
	model.add(balance == netLoad);

	balance.end();
	netLoad.end();
}
//
// ------------------- Spinning Reserve Constraints -------------------
for (int t = 0; t < StepNum; ++t)
{
	/* Nếu vẫn còn lưới → KHÔNG đặt ràng buộc dự phòng */
	if (GridMaintainance1[t] == 1)   // 1 = on-grid, 0 = off-grid
		continue;                    // sang bước tiếp theo

	/* ====== 1. SR_avail ====== */
	IloExpr SR_avail(env);

	/* 1.a – Diesel Gen:  Pmax · U */
	for (int g = 0; g < NumDG; ++g)
		SR_avail += P_DGMax[g] * Committed[g][t];

	/* 1.b – BESS:  P_SR đã ràng buộc ≤ min{…} ở trên */
	for (int b = 0; b < NumBESS; ++b)
		SR_avail += P_SR[b][t];

	/* ====== 2. SR_need ====== */
	IloExpr SR_need(env);

	/* 2.a – Load */
	SR_need += (Load1m_forcast[t] + Load2m_forcast[t] + Load3m_forcast[t]);

	/* 2.b – Load Shedding */
	SR_need -= (P_LoadShed1[t] + P_LoadShed2[t] + P_LoadShed3[t]);

	/* 2.c – RE */
	SR_need -= (P_RE_PV[t] + P_RE_Wind[t]);

	/* 2.d – + “Required SR” */
	SR_need += 0;

	/* ====== 3. Ràng buộc đủ dự phòng ====== */
	model.add(SR_avail >= SR_need);

	/* ====== 4. Phải có ÍT NHẤT 1 phần tử tạo tần số ====== */
	IloExpr gridForm(env);

	/* Diesel Gen online */
	for (int g = 0; g < NumDG; ++g)
		gridForm += Committed[g][t] * P_DGMax[g];

	/* Hoặc BESS có SoC >= SoC_rev  (O_BESS = 1) */
	for (int b = 0; b < NumBESS; ++b)
		gridForm += O_BESS[b][t];

	model.add(gridForm > 0);

	/* ====== 5. Giải phóng bộ nhớ ====== */
	SR_avail.end();
	SR_need.end();
	gridForm.end();
}
//
// ------------------- Total Cost Function -------------------
IloExpr totalCost(env);

totalCost += totalGridBuyCost + totalGridSellCost;   // lưới
totalCost += totalFuelCost;							 // DG fuel
totalCost += totalBESSCost;							 // BESS (xả + SR)
totalCost += Pv_Cur_cost + Wind_Cur_cost;			 // Curtail RE
totalCost += totalLoadShedCost;						 // Shed load

/* hệ số phạt ($/kW) – chỉnh tùy ý, càng lớn càng “mượt” */
const double SMOOTH_W = 1e-6;

for (int b = 0; b < NumBESS; ++b)
	for (int t = 1; t < StepNum; ++t)
	{
		totalCost += SMOOTH_W * 
			(Ramp_dch[b][t]);
		totalCost += SMOOTH_W * 
			(Ramp_ch[b][t]);
	}

model.add(IloMinimize(env, totalCost));

IloCplex cplex(model);

// Model parameters
//for (int i = 0; i < NumDG; i++) {
//	for (int j = 0; j < StepNum; j++) {
//		cplex.setPriority(Committed[i][j], j);
//		cplex.setPriority(StartUp[i][j], j);
//		cplex.setPriority(ShutDown[i][j], j);
//	}
//}

//GRID 
// P_GridBuy, P_GridSell, P_GridBuyExtra, P_GridSellExtra
vector<double> O_PowerGridBuy(StepNum, 0);
vector<double> O_PowerGridSell(StepNum, 0);
vector<double> O_PowerGridBuyExtra(StepNum, 0); // P_GridBuyExtra
vector<double> O_PowerGridSellExtra(StepNum, 0); // P_GridSellExtra

// DG
vector<vector<int>> O_DGCommitted(NumDG, vector<int>(StepNum, 0));
vector<vector<double>> O_PowerDG(NumDG, vector<double>(StepNum, 0));

// BESS
vector<vector<double>> O_SOC(NumBESS, vector<double>(StepNum, 0));
vector<vector<double>> O_PowerCharge(NumBESS, vector<double>(StepNum, 0));
vector<vector<double>> O_PowerDischarge(NumBESS, vector<double>(StepNum, 0));
vector<vector<int>>    O_U_BESS_dch(NumBESS, vector<int>(StepNum, 0));   // 1 → đang xả
vector<vector<int>>    O_U_BESS_ch(NumBESS, vector<int>(StepNum, 0));   // 1 → đang nạp
vector<vector<int>>    O_O_BESS(NumBESS, vector<int>(StepNum, 0));   // 1 → được tính SR
vector<vector<double>> O_P_SR(NumBESS, vector<double>(StepNum, 0));/* (kW) công suất SR */


// RE
vector<double> O_P_RE_PV(StepNum, 0);
vector<double> O_P_RE_Wind(StepNum, 0);
vector<double> O_P_RE_PV_Cur(StepNum, 0.0);   // (kW) PV curtail
vector<double> O_P_RE_Wind_Cur(StepNum, 0.0);   // (kW) Wind curtail

// Load Sheding
vector<double> O_LoadShed1(StepNum, 0.0);   // Type 1
vector<double> O_LoadShed2(StepNum, 0.0);   // Type 2
vector<double> O_LoadShed3(StepNum, 0.0);   // Type 3

// ------------------- Solve and catch output data -------------------
try
{
	/* ---------- 1. Xuất .lp (debug) & đặt tham số ---------- */
	std::string modelOutFile = Output_path + "/Model_Microgrid_ED.lp";
	cplex.exportModel(modelOutFile.c_str());

	//cplex.setParam(IloCplex::Param::TimeLimit, 2000);   // 200 s
	//cplex.setParam(IloCplex::Param::MIP::Display, 2); // hiện log chi tiết
	//cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 1e-5);

	/* 2. Nới lỏng độ chính xác */
	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 5e-3);   // 0.5 %
	cplex.setParam(IloCplex::Param::MIP::Tolerances::Integrality, 1e-5);

	/* 3. Hạn chế độ phức tạp của cây nhánh-cận */
	cplex.setParam(IloCplex::Param::MIP::Limits::Nodes, 200000);   // tối đa 20 k nodes
	cplex.setParam(IloCplex::Param::MIP::Limits::CutsFactor, 4);  // giảm số lượng cắt
	cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, 10); // tăng heuristic

	/* ---------- 2. Giải ---------- */
	if (!cplex.solve())
		throw std::runtime_error("CPLEX: no feasible solution");

	/* ---------- 3. In trạng thái & hàm mục tiêu ---------- */
	std::cout << "Solution status : " << cplex.getStatus() << '\n'
		<< std::fixed << std::setprecision(5)
		<< "Objective value : " << cplex.getObjValue() << '\n';

	//* ---------- 4. Output get ---------- */

	 /* ---- 4.1  GRID ---- */
	for (int t = 0; t < StepNum; ++t) {
		O_PowerGridBuy[t] = cplex.getValue(P_GridBuy[t]);
		O_PowerGridSell[t] = cplex.getValue(P_GridSell[t]);
		O_PowerGridBuyExtra[t] = cplex.getValue(P_GridBuyExtra[t]);
		O_PowerGridSellExtra[t] = cplex.getValue(P_GridSellExtra[t]);
	}

	/* ---- 4.2  DG ---- */
	for (int g = 0; g < NumDG; ++g)
		for (int t = 0; t < StepNum; ++t)
		{
			O_DGCommitted[g][t] = static_cast<int>(cplex.getValue(Committed[g][t]));
			double p = cplex.getValue(PowerDG[g][t]);
			O_PowerDG[g][t] = (p < 1e-6 ? 0.0 : p);           //  gọt nhiễu số
		}

	/* ---- 4.3  BESS ---- */
	for (int b = 0; b < NumBESS; ++b)
		for (int t = 0; t < StepNum; ++t)
		{
			O_SOC[b][t] = cplex.getValue(SoC[b][t]);
			O_PowerDischarge[b][t] = cplex.getValue(P_BESS_dch[b][t]);
			O_PowerCharge[b][t] = cplex.getValue(P_BESS_ch[b][t]);

			O_U_BESS_dch[b][t] = static_cast<int>(cplex.getValue(U_BESS_dch[b][t]));
			O_U_BESS_ch[b][t] = static_cast<int>(cplex.getValue(U_BESS_ch[b][t]));
			O_O_BESS[b][t] = static_cast<int>(cplex.getValue(O_BESS[b][t]));

			O_P_SR[b][t] = cplex.getValue(P_SR[b][t]);
			/*O_Ramp_BESS_DCh[b][t] = cplex.getValue(Ramp_dch[b][t]);
			O_Ramp_BESS_CH[b][t] = cplex.getValue(Ramp_ch[b][t]);*/
		}

	/* ---- 4.4  NĂNG LƯỢNG TÁI TẠO ---- */
	for (int t = 0; t < StepNum; ++t)
	{
		O_P_RE_PV[t] = cplex.getValue(P_RE_PV[t]);
		O_P_RE_Wind[t] = cplex.getValue(P_RE_Wind[t]);

		O_P_RE_PV_Cur[t] = cplex.getValue(P_RE_PV_Cur[t]);
		O_P_RE_Wind_Cur[t] = cplex.getValue(P_RE_Wind_Cur[t]);
	}

	/* ---- 4.5  LOAD SHEDDING ---- */
	for (int t = 0; t < StepNum; ++t)
	{
		O_LoadShed1[t] = cplex.getValue(P_LoadShed1[t]);
		O_LoadShed2[t] = cplex.getValue(P_LoadShed2[t]);
		O_LoadShed3[t] = cplex.getValue(P_LoadShed3[t]);
	}
}
/* ---------- 5. Bắt ngoại lệ ---------- */
catch (const IloException& e)
{
	std::cerr << "CPLEX error : " << e.getMessage() << '\n';
}
catch (const std::exception& e)
{
	std::cerr << "Error       : " << e.what() << '\n';
}
catch (...)
{
	std::cerr << "Unknown error\n";
}

// ----------------------- OUTPUT DATA to CSV -------------------
{
	/* ---- 6.1  Chuỗi thời gian “HH:MM:SS” ---- */
	std::vector<std::string> periods;
	{
		const int dayMin = 24 * 60;
		const int stepMin = static_cast<int>(StepReSol * 60);   // Δt phút
		const int totStep = dayMin / stepMin;
		for (int k = 0; k < totStep; ++k)
		{
			int hh = (k * stepMin) / 60;
			int mm = (k * stepMin) % 60;
			std::ostringstream oss;
			oss << std::setw(2) << std::setfill('0') << hh << ':'
				<< std::setw(2) << std::setfill('0') << mm << ":00";
			periods.emplace_back(oss.str());
		}
	}

	/* ---- 6.2  Khai báo tên thiết bị ---- */
	std::vector<std::string> DG_name = { "DG1","DG2","DG3","DG4" };      // tuỳ NumDG
	std::vector<std::string> BESS_name = { "BESS1","BESS2" };              // tuỳ NumBESS

	/* ---- 6.3  Mở file ---- */
	std::ofstream csv(Output_path + "/Output_Microgrid_ED.csv");
	if (!csv.is_open())
		throw std::runtime_error("Cannot create CSV file");

	/* ---- 6.4  HEADER ---- */
	std::ostringstream hdr;
	hdr << "Time,"
		<< "Load (kW),"
		<< "Grid Buy Coef ($/kWh),"
		<< "Grid Buy (kW),"
		<< "Grid Sell (kW),"
		<< "Grid BuyExtra (kW),"
		<< "Grid SellExtra (kW),"
		;


	/* RE */
	hdr << "PV Used (kW),Wind Used (kW),"
		<< "PV Curtail (kW),Wind Curtail (kW),";

	/* Load-shed */
	hdr << "LoadShedding_1 (kW),LoadShedding_2 (kW),LoadShedding_3 (kW),";

	/* DG power */
	for (int g = 0; g < NumDG; ++g) hdr << DG_name[g] << " (kW),";
	/* DG status / start / stop */
	for (int g = 0; g < NumDG; ++g) hdr << DG_name[g] << " U,";

	/* BESS charge/discharge/SOC */
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " Charge (kW),";
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " Discharge (kW),";
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " SOC (%),";

	/* BESS logic & SR */
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " U_dch,";
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " U_ch,";
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " O_SR,";
	for (int b = 0; b < NumBESS; ++b) hdr << BESS_name[b] << " P_SR (kW),";

	csv << hdr.str() << '\n';

	/* ---- 6.5  DỮ LIỆU THEO TỪNG STEP ---- */
	for (int t = 0; t < StepNum; ++t)
	{
		std::ostringstream row;
		double loadTotal = Load1m_forcast[t] + Load2m_forcast[t] + Load3m_forcast[t];

		/* Thời gian + tổng phụ tải + hệ số giá mua lưới */
		row << periods[t] << ','
			<< loadTotal << ','
			<< GridBuyCost1[t] << ','
			<< O_PowerGridBuy[t] << ','
			<< O_PowerGridSell[t] << ','
			<< O_PowerGridBuyExtra[t] << ','
			<< O_PowerGridSellExtra[t] << ',';

		/* RE */
		row << O_P_RE_PV[t] << ','
			<< O_P_RE_Wind[t] << ','
			<< O_P_RE_PV_Cur[t] << ','
			<< O_P_RE_Wind_Cur[t] << ',';

		/* Load shedding */
		row << O_LoadShed1[t] << ','
			<< O_LoadShed2[t] << ','
			<< O_LoadShed3[t] << ',';

		/* DG power */
		for (int g = 0; g < NumDG; ++g) row << O_PowerDG[g][t] << ',';
		/* DG logic */
		for (int g = 0; g < NumDG; ++g) row << O_DGCommitted[g][t] << ',';

		/* BESS charge / discharge / SOC */
		for (int b = 0; b < NumBESS; ++b) row << O_PowerCharge[b][t] << ',';
		for (int b = 0; b < NumBESS; ++b) row << O_PowerDischarge[b][t] << ',';
		for (int b = 0; b < NumBESS; ++b) row << O_SOC[b][t] << ',';

		/* BESS logic & SR */
		for (int b = 0; b < NumBESS; ++b) row << O_U_BESS_dch[b][t] << ',';
		for (int b = 0; b < NumBESS; ++b) row << O_U_BESS_ch[b][t] << ',';
		for (int b = 0; b < NumBESS; ++b) row << O_O_BESS[b][t] << ',';
		for (int b = 0; b < NumBESS; ++b) row << O_P_SR[b][t] << ',';

		csv << row.str() << '\n';
	}
	csv.close();
	env.end();    //  Giải phóng bộ nhớ CPLEX / IloEnv
}


// END PROGRAM HERE
}