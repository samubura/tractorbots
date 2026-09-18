// Agent tractor in project smartFarming

/* Initial beliefs and rules */

water(11).
battery(100).

/* Initial goals */
!start.



/* Plans */

+!start : true <- .print("Tractor agent is ready!").

+auction(S)[source(A)] :true
<-
	.print("Received auction notificationf for ", S);
	?canIrrigate(S, true);
	!bidIfPossible(A,S).

+!bidIfPossible(A,S):canIrrigate(S,C) & C
<-
	.print("Sending bid to ", A);
	-+hasBid(S,100);
	.send(A, tell, bid(S,100)).

-!bidIfPossible(A,S)
<-
	.print("Cant bid for ", S).
		
+!irrigate(Section)[source(Ag)]:true
<-	?canIrrigate(Section, true)
	!conductIrrigation(S).

+!conductIrrigation(S):canIrrigate(S, C) & C
<- .print("Conducting irrigation").

+?canIrrigate(Section, Answer):water(W) & battery(B)
<-
	.print("Obtaining belief about canIrrigate W=", W, " B=", B);
	evaluateState(Section,W,B,A); //Ask KG if I can irrigate
	+canIrrigate(Section,A).

-!irrigate(Section)[source(Ag)]:true
<-.print("cant irrigate").

-!conductIrrigation:true
<-.print("cant conduct irrigation"). //Tell KG that I am irrigating

{ include("$jacamoJar/templates/common-cartago.asl") }
{ include("$jacamoJar/templates/common-moise.asl") }
// uncomment the include below to have an agent compliant with its organisation
//{ include("$moiseJar/asl/org-obedient.asl") }
