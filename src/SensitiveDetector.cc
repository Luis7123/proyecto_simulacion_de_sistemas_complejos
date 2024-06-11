#include "SensitiveDetector.hh"
#include "G4AnalysisManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4TouchableHistory.hh"
#include "G4Step.hh"
#include "G4ios.hh"
#include "G4RootAnalysisManager.hh"

SensitiveDetector::SensitiveDetector(const G4String& name):
 G4VSensitiveDetector(name)
{}

SensitiveDetector::~SensitiveDetector()
{}

void SensitiveDetector::Initialize(G4HCofThisEvent* hce)
{}

G4bool SensitiveDetector::ProcessHits(G4Step* step,
                                 G4TouchableHistory* history)
{
  G4double edep = step->GetTotalEnergyDeposit();

  G4Track* track = step->GetTrack();
  G4double trackLenght = track->GetTrackLength();

  G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
    analysisManager->FillH1(0, edep);
    analysisManager->FillH1(1, trackLenght);

  return true;
}

void SensitiveDetector::EndOfEvent(G4HCofThisEvent* hce)
{

}
