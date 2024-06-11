
#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"

#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4VisAttributes.hh"
#include "G4Color.hh"
#include "G4UnionSolid.hh"
#include "G4Material.hh"
#include "G4SDManager.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4Cons.hh"
#include "globals.hh"
#include "G4SubtractionSolid.hh"
#include "SensitiveDetector.hh"

#include "G4UniformElectricField.hh"
#include "G4FieldManager.hh"
#include "G4EqMagElectricField.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4ClassicalRK4.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4ChordFinder.hh"

namespace med_linac
{
	G4VPhysicalVolume* DetectorConstruction::Construct()
	{
		// construct our detectors here

        // Get nist material manager
        G4NistManager* nist = G4NistManager::Instance();


		// Start with constructing the world:
        G4double worldSize = 1.5 * m;
        G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
	G4Material* soft = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
	G4Material* skeleton = nist->FindOrBuildMaterial("G4_B-100_BONE");
	G4Material* muscle = nist->FindOrBuildMaterial("G4_MUSCLE_WITHOUT_SUCROSE");
	G4Material* copper = nist->FindOrBuildMaterial("G4_Cu");
        auto sphereWorld = new G4Sphere(
            "solidWorld",
            0,
            worldSize,
            0,
            2 * CLHEP::pi,
            0,
            CLHEP::pi
        );

        auto boxWorld = new G4Box(
            "solidWorld",
            worldSize / 2,
            worldSize / 2,
            worldSize
        );


        auto logicWorld = new G4LogicalVolume(boxWorld,
            vacuum, 
            "logicWorld");


        auto physWorld = new G4PVPlacement(nullptr,
            G4ThreeVector(), 
            logicWorld, 
            "physWorld", 
            nullptr, 
            false, 
            0);


	//HEAD
	G4Color pink(1,0.89,0.8);
	G4VSolid* headS = new G4Sphere("Head_solid",0.,15.*cm,
		                 0.,360.*deg,0,180.*deg);

	G4LogicalVolume* logicHead0 
	= new G4LogicalVolume(headS,muscle,"Head_logical");

	G4VPhysicalVolume* head
	= new G4PVPlacement(0,
		       G4ThreeVector(0.,0.,0),
		       logicHead0,
		       "Head_physical",
		       logicWorld,
		       false,
		       0);

	G4VisAttributes* headVisAtt = new G4VisAttributes(pink);
	logicHead0->SetVisAttributes(headVisAtt);
		
	//cerebro
	

	G4Color red(1.0,0.,0.);
	G4VSolid* brainS = new G4Sphere("Brain_solid",0.,10.*cm,
		                0.,270.*deg,0,180.*deg);

	G4LogicalVolume* logicBrain
	= new G4LogicalVolume(brainS,soft,"Brain_logical");

	G4VPhysicalVolume* brain
	= new G4PVPlacement(0,
		       G4ThreeVector(),
		       logicBrain,
		       "Brain_physical",
		       logicHead0,
		       false,
		       0);

	G4VisAttributes* brainVisAtt = new G4VisAttributes(red);
	brainVisAtt->SetForceSolid(true);
	brainVisAtt->SetVisibility(true);
	logicBrain->SetVisAttributes(brainVisAtt);
	
	
		
        G4double accelerationVoltage = 100 * kilovolt; // Por ejemplo, 100 kV


        // Next, create a 'linac head' object to contain all the radiation generation stuff
        G4double linacHeadThicknessXY = 15 * cm;
        G4double linacHeadThicknessZ = 15 * cm;

        G4ThreeVector linacHeadPos = G4ThreeVector(0 * cm , 0 * cm, -1 * m);

        G4double headPhi = 0;
        G4double headTheta = CLHEP::pi / 4;
        G4double headPsi = 0;
        G4RotationMatrix* linacHeadRotation = new G4RotationMatrix(headPhi, headTheta, headPsi);

        G4Box* solidHead = new G4Box("solidHead", linacHeadThicknessXY, linacHeadThicknessXY, linacHeadThicknessZ);
        G4LogicalVolume* logicHead = new G4LogicalVolume(solidHead, vacuum, "logicHead");
        G4VPhysicalVolume* physHead = new G4PVPlacement(
            0,
            linacHeadPos,
            logicHead,
            "physHead",
            logicWorld,
            false,
            0);

        // set the member variable so we can get this volume in other parts of the program
        fLinacHead = physHead;


        // create a place for the particle gun to shoot from
        G4double particleGunAnchorThickness = 1 * mm;
        G4Box* solidParticleGunAnchor = new G4Box(
            "solidParticleGunAnchor", 
            particleGunAnchorThickness, 
            particleGunAnchorThickness, 
            particleGunAnchorThickness);

        G4ThreeVector particleGunAnchor1Pos = G4ThreeVector(0, 0, -linacHeadThicknessZ + 2 * cm);

        G4LogicalVolume* logicParticleGunAnchor1 = new G4LogicalVolume(solidParticleGunAnchor, vacuum, "logicParticleGunAnchor1");
        G4VPhysicalVolume* physAnchor1 = new G4PVPlacement(
            nullptr,
            particleGunAnchor1Pos,
            logicParticleGunAnchor1,
            "physParticleGunAnchor1",
            logicHead,
            false,
            0);


        // Set our member variables for the gun anchors:
        fParticleGunAnchor1 = physAnchor1;


        // create our tungsten target
        G4Material* tungsten = nist->FindOrBuildMaterial("G4_W");

        G4double innerTargetRadius = 0.0;
        G4double outerTargetRadius = 1.5 * cm;
        G4double targetThickness = 1 * mm;

        G4Tubs* solidTarget = new G4Tubs("Target",
            innerTargetRadius,
            outerTargetRadius,
            targetThickness / 2.0,
            0.0,
            360.0 * deg);

        G4LogicalVolume* logicTarget = new G4LogicalVolume(solidTarget, 
            tungsten, 
            "Target");

        // target position and rotation
        G4double targetZ = -linacHeadThicknessZ + 5 * cm;
        G4ThreeVector targetPos = G4ThreeVector(0, 0, targetZ); // 0,0,0
        G4RotationMatrix* targetRotation = new G4RotationMatrix();

        // place the target in the world
        new G4PVPlacement(
            targetRotation, 
            targetPos, 
            logicTarget, 
            "Target",
            logicHead, 
            false, 
            0);
	
        // create a graphite absorber to absorb electrons
	
	G4Material* graphite = nist->FindOrBuildMaterial("G4_GRAPHITE");

        G4double innerAbsorberRadius = 0.0;
        G4double outerAbsorberRadius = 1.5 * cm;
        G4double absorberThickness = 1.5 * cm;

        G4Tubs* solidAbsorber = new G4Tubs("Absorber",
            innerAbsorberRadius,
            outerAbsorberRadius,
            absorberThickness / 2.0,
            0.0,
            360.0 * deg);

        G4LogicalVolume* logicAbsorber = new G4LogicalVolume(solidAbsorber,
            graphite,
            "Absorber");

        // absorber position and rotation
        G4double absorberZ = targetPos.getZ() + (absorberThickness / 2);
        G4ThreeVector absorberPos = G4ThreeVector(0.0, 0.0, absorberZ);
        G4RotationMatrix* absorberRot = new G4RotationMatrix();

        // place the absorber
        new G4PVPlacement(absorberRot,
            absorberPos,
            logicAbsorber,
            "Absorber",
            logicHead,
            false,
            0);



        // create tungsten collimator
        G4double innerColRadius = 4.0 * cm;
        G4double outerColRadius = 12. * cm;
        G4double colThickness = 10 * cm;

        G4Tubs* solidCol = new G4Tubs("Collimator",
            innerColRadius,
            outerColRadius,
            colThickness / 2.0,
            0.0,
            360.0 * deg);
        G4LogicalVolume* logicCol = new G4LogicalVolume(solidCol,
            tungsten,
            "Collimator");

        // collimator position and rotation
        G4double colZ =
            absorberZ
            + (absorberThickness / 2)
            + (colThickness / 2);

        G4ThreeVector colPos = G4ThreeVector(0, 0, colZ);

        G4RotationMatrix* colRotation = new G4RotationMatrix();

        // place the collimator in the world
        new G4PVPlacement(0,
            colPos,
            logicCol,
            "Collimator",
            logicHead,
            false,
            0);



	// Definir los electrodos
        G4double electrodeThickness = 1.0 * mm;
        G4double electrodeSizeXY = outerColRadius * 2;

        G4Box* solidElectrode = new G4Box("Electrode", electrodeSizeXY / 2, electrodeSizeXY / 2, electrodeThickness / 2);

        G4LogicalVolume* logicElectrode = new G4LogicalVolume(solidElectrode, copper, "Electrode");

        // Crear una matriz de rotación para girar los electrodos 90 grados alrededor del eje Y
        G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
        rotationMatrix->rotateY(90.0 * deg);

        // Posicionar los electrodos dentro del colimador con la rotación aplicada
        G4double electrode1Z = colZ + (colThickness );
        G4double electrode2Z = colZ + (colThickness );

        new G4PVPlacement(rotationMatrix, G4ThreeVector(-5*cm, 0, electrode1Z), logicElectrode, "Electrode1", logicHead, false, 0);
        new G4PVPlacement(rotationMatrix, G4ThreeVector(5*cm, 0, electrode2Z), logicElectrode, "Electrode2", logicHead, false, 0);

        // Crear y asignar el campo eléctrico
        G4ThreeVector fieldVector(0.0, 0.0, 100.0 * kilovolt / cm); // Definir la fuerza y dirección del campo
        G4UniformElectricField* electricField = new G4UniformElectricField(fieldVector);

        G4EqMagElectricField* equation = new G4EqMagElectricField(electricField);

        G4MagIntegratorStepper* stepper = new G4ClassicalRK4(equation, 8);
        G4double minStep = 1.0 * mm;
        G4MagInt_Driver* driver = new G4MagInt_Driver(minStep, stepper, stepper->GetNumberOfVariables());
        G4ChordFinder* chordFinder = new G4ChordFinder(driver);

        G4FieldManager* fieldManager = new G4FieldManager(electricField);
        fieldManager->SetChordFinder(chordFinder);

        // Aplicar el campo eléctrico únicamente al volumen entre los electrodos
        logicCol->SetFieldManager(fieldManager, true);

        // Atributos visuales
        G4VisAttributes* electrodeVisAtt = new G4VisAttributes(G4Color::Yellow());
        electrodeVisAtt->SetForceSolid(true);
        logicElectrode->SetVisAttributes(electrodeVisAtt);
        
	//Sensitive Detector.
	G4VSensitiveDetector* head_1 = new SensitiveDetector("headSD");
        logicHead0->SetSensitiveDetector(head_1);


        // finish by returning the world
        return physWorld;
    }

}
