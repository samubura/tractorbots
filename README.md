# Smart Farming

## Overview
The smart farming project at [Interactions]{https://ics.unisg.ch/chair-interactions-mayer/#landing} group is aimed to bring together semantic description of the farming environment, its smart machines like automated tractors, and software agents to create a testing an proving ground for several research work in the group. We partition our work (for the sake of project management) into following regions:

1. Semantic modeling of the smart farm environment
2. Multi-agent system (MAS) which uses the knowledge graph to both understand the environment, and enable agents to plan and interact to achieve the goals.
3. Building in mechanisms to facilitate explaination of the systems behaviour
4. Use HCI means such as AR/MR to enable farmers to understand the farm environment

## Semantic Model
We have a draft version of the SmartFarming ontology (SmaFaOnt) which attempts to describe the farmland, the crops, the agricultural processes required for such crops, the farm machinery, and some basic **features of interest** (need to clarify this term!).  We intend to use WoT Thing Description to describe the affordances of artefacts (includes things which "active" like the tractors and also "passive" like the farmland).

## MAS
The affairs of the farm shall be conducted by MAS programmed using JaCaMo. Danai and Andrei will figure this out.

## Explainability
If the farmer asks "why is tractorbot spock irrigating field section 1?", then Sanjiv's system will get some semantically annotated logs which Danai will kindly generate, and say that it is doing so because land is too dry for wheat crop and that tractorbot Uhura is running out of battery.

## Visualizing
If you then put on Kenan's glasses and look at Uhura, you will not only know her battery level, but can also look at the field and see how much she can cover before having to recharge.

