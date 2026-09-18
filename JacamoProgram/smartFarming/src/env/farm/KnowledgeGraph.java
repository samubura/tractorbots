// CArtAgO artifact code for project smartFarming

package farm;

import cartago.*;

public class KnowledgeGraph extends Artifact {
	void init(int initialValue) {
		//defineObsProperty("soilCondition", initialValue);
		defineObsProperty("sectionNeedsIrrigation", 0);
	}

	@OPERATION
	void updateSoilCondition(int section, int soilMoisture) {
		ObsProperty prop = getObsProperty("sectionNeedsIrrigation");
		prop.updateValue(section);
		signal("stateChange");
	}
	
	@OPERATION
	void evaluateState(int section, int water, int battery, OpFeedbackParam<Boolean> result) {
		System.out.println("Got request");
		result.set(water > 10);
	}
	
	@OPERATION
	void updateIrrigationAction(int section, String tractorId) {

	}	
}

