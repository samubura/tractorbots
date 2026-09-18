// Agent sample_agent in project smartFarming

/* Initial beliefs and rules */

/* Initial goals */
!maintaincrophealth.

/* Plans */
	
+!maintaincrophealth:true
<-
	.broadcast(achieve, survey);
	.print("Manager monitoring the farm").

@deployIrrigatorsPlan
+sectionNeedsIrrigation(Section): Section > 0 //This is an event raised by KG
<-
	.send(t1, tell, water(0));
	.broadcast(tell, auction(Section));
	.at("now + 10 seconds", {+!decide(Section)});
	.print("Asked for bids to irrigate section ", Section).

+bid(S,V)[source(A)]
<- .print("Source ", A, " has bid for ", S, " with value ", V).

+!decide(S): .findall(b(V,A),bid(S,V)[source(A)],L)
<- 
	.min(L,b(V,W));
	.print("Winner for ", S, " is ",W," with ", V, ". Options=",L);
	.send(W, achieve, irrigate(S));
	.broadcast(untell, auction(S)).

{ include("$jacamoJar/templates/common-cartago.asl") }
{ include("$jacamoJar/templates/common-moise.asl") }

// uncomment the include below to have an agent compliant with its organisation
//{ include("$moiseJar/asl/org-obedient.asl") }
