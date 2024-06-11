
#include "RunAction.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"


#include "G4UserRunAction.hh"
#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4RootAnalysisManager.hh"

namespace med_linac {
	RunAction::RunAction() {

	}

	RunAction::~RunAction() {
	}

	void RunAction::BeginOfRunAction(const G4Run* aRun) {
		// start time
		fTimer.Start();


		G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
 	 	G4String fileName = "Linac.root";
	  	analysisManager->OpenFile(fileName);

  		analysisManager->CreateH1("Energia", "Histograma de Energía", 400.,0.,400.);
  		analysisManager->CreateH1("Trayectoria", "Histograma de trayectoria", 400.,0.,400.);
	}

	void RunAction::EndOfRunAction(const G4Run* aRun) {

		// end time
		fTimer.Stop();

		// print out the time it took
		if (IsMaster()) {
			PrintTime();
		}
		G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
		
		analysisManager->Write();
		analysisManager->CloseFile();

	}

	void RunAction::PrintTime() {
		auto time = fTimer.GetRealElapsed();

		G4cout
			<< "Elapsed time: "
			<< time
			<< " Seconds."
			<< G4endl;

	}
}

