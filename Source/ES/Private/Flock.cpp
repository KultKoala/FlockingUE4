// Landon Byrd
// Flocking Implementation


#include "Flock.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Async/ParallelFor.h"
#include "..\Public\Flock.h"

// Sets default values
AFlock::AFlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//set up flock defaults
	FlockSize = 50;
	kCoh = .4;
	kSep = .8;
	kAlign = .05;

	rCohesion = 1000;
	rSeparation = 500;
	rAlignment = 150;
	FlockAbsoluteBounds = 100000;

	maxAccel = 2;
	maxVel = 15;

	//create instanced static mesh component for creating instanced flock members
	FlockInstancedMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstanceComp"));
}

void AFlock::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//set up flock mesh and material
	FlockMesh->SetMaterial(0, FlockMaterial);
	FlockInstancedMeshComp->SetStaticMesh(FlockMesh);

	//initalize back and forward buffer for calculating flock movement 
	PreviousAgents.Init(FTInfo(), FlockSize);
	CurrentAgents.Init(FTInfo(), FlockSize);

	//calculate initial values for flock member
	for (uint32 i = 0; i < FlockSize; i++) {

		//find random location in user defined range and set mesh location
		FVector RandomLocation = FMath::RandPointInBox(FBox(FVector(-1000, -1000, -1000), FVector(1000, 1000, 1000)));

		//store id for access to instance in component
		PreviousAgents[i].instanceId = FlockInstancedMeshComp->AddInstance(FTransform(RandomLocation));
		CurrentAgents[i].instanceId = PreviousAgents[i].instanceId;
		PreviousAgents[i].position = RandomLocation;
	}
}

// Called when the game starts or when spawned
void AFlock::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//calculate flock positions
	FlockingCalculations(DeltaTime);

	//pass updates to instanced meshes
	UpdateFlockMeshes();

}

//run calculations for entire flock in parallel
void AFlock::FlockingCalculations(float DTime)
{
	//calculate position of each member in parallel
	ParallelFor (FlockSize, [&](uint32 i) {
		FlockMemberCalculations(i,DTime);
		});
}

//run calculations for individual member of flock
void AFlock::FlockMemberCalculations(uint32 FlockIndex, float DTime)
{
	//vectors to keep track of coh, sep, and alg influence on given flock member
	FVector Cohesion(FVector::ZeroVector), Separation(FVector::ZeroVector), Alignment(FVector::ZeroVector);

	//keep track of found flock with radius
	int32 CohesionCnt = 0, SeparationCnt = 0, AlignmentCnt = 0;

	for (uint32 i = 0; i < FlockSize; i++) {
		if (i != FlockIndex) {

			float MemberDistance = FVector::Distance(PreviousAgents[FlockIndex].position, PreviousAgents[i].position);

			//falls within cohesion range
			if (MemberDistance < rCohesion) {
				Cohesion += PreviousAgents[i].position;
				CohesionCnt++;
			}

			//falls within separation range
			if (MemberDistance < rSeparation) {
				Separation += PreviousAgents[i].position - PreviousAgents[FlockIndex].position;
				SeparationCnt++;
			}

			//falls within alignment range
			if (MemberDistance < rAlignment) {
				Alignment += PreviousAgents[i].velocity;
				AlignmentCnt++;
			}
		}

		//member found in cohesion range
		if (CohesionCnt > 0) {
			Cohesion /= CohesionCnt;
			Cohesion -= PreviousAgents[FlockIndex].position;
			Cohesion.Normalize();
		}

		//member found in separation range
		if (SeparationCnt > 0) {
			Separation /= SeparationCnt;
			Separation *= -1.f;
			Separation.Normalize();
		}

		//member found in alignment range
		if (AlignmentCnt > 0) {
			Alignment /= AlignmentCnt;
			Alignment.Normalize();
		}

		//calculate updated values and pass to front buffer
		CurrentAgents[FlockIndex].acceleration = (Cohesion * kCoh + Separation * kSep + Alignment * kAlign).GetClampedToMaxSize(maxAccel);
		CurrentAgents[FlockIndex].velocity += CurrentAgents[FlockIndex].acceleration * DTime;
		CurrentAgents[FlockIndex].velocity = CurrentAgents[FlockIndex].velocity.GetClampedToMaxSize(maxVel);
		CurrentAgents[FlockIndex].position += CurrentAgents[FlockIndex].velocity * DTime;

	}
}

void AFlock::UpdateFlockMeshes()
{
	for(uint32 i = 0; i<FlockSize;i++)
	{
		//convert position calculated for flock member to transform
		FTransform InstanceTransform = FTransform(CurrentAgents[i].position);
		
		//calculate direction for flock member
		FVector Direction = CurrentAgents[i].velocity;
		Direction.Normalize();
		InstanceTransform.SetRotation(Direction.Rotation().Add(0.f, -90.f, 0.f).Quaternion());

		//pass in updates to given mesh
		FlockInstancedMeshComp->UpdateInstanceTransform(CurrentAgents[i].instanceId, InstanceTransform, false, false);
	}

	//tell render thread to render updates to instance meshes
	FlockInstancedMeshComp->MarkRenderStateDirty();

	//swap buffer
	PreviousAgents = CurrentAgents;
}

