#ifndef SensitiveDetector_hh
#define SensitiveDetector_hh

#include "G4VSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"

class G4Step;
class G4CofThisEvent;
class G4TouchableHistory;

class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    SensitiveDetector(const G4String& name);
    virtual ~SensitiveDetector();

    virtual void Initialize(G4HCofThisEvent* hitsCollection);
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
    virtual void EndOfEvent(G4HCofThisEvent* hitsCollection);

 };

#endif
