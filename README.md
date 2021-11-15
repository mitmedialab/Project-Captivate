# Project Captivate
## A Wearable Platform for Across-Context Measurement and Actuation 

### Note: experimental branch with new packet sending architecture
(1) Each sensor channel sends its own packet versus the original architecture of interleaving different sensor data into one packet type
(2) Once turned on, the glasses send data via BLE with the following definition:
 \tService: 19ed82aeed214c9d4145228e80FE0000 
 \tCharacteristic: 19ed82aeed214c9d4145228e81FE0000  

### Across Contexts

Project Captivate have been designed to serve as an easy and reliable 
platform to measure physiology in the real world.  We've designed it to 
be a tool for researchers, a means to understand attention and 
engagement, and a scalable means to control responsive ecosystems. 

### A Tool for Researchers
Smartglasses are increasingly popular because the face is an ideal 
location for continuous monitoring of environmental and physiological 
signals. Unfortunately, no standard smartglasses platforms offer easy 
access to physiological data.  That leaves researchers constrained by 
their own quick—and usually bulky—designs that make people 
self-conscious and uncomfortable, altering their behavior and preventing 
long-term naturalistic studies.The Captivate platform addresses these 
concerns; it saves researchers design time and gives them easy access to 
data in a form factor that doesn't stand out.We spent a summer in 
Shenzhen, China, learning from eyeglass manufacturers on how best to 
integrate sensors that track physiology events across your face, head 
movements, and location in a form-factor that is similar to a 
traditional pair of glasses. 

### Measuring Attention Across Contexts

We are using the Captivate platform study a user’s cognitive state, 
specifically cognitive loading and overall attention.  Many lab-based 
behavioral studies fail to replicate in real-world contexts.  Because of 
the deep engineering work behind these glasses, we are capable of 
studying people naturally—all day long, in their real lives.  Combined 
with new probabilistic modeling tools, we can make much more reliable 
predictions about a user's cognitive state in real scenarios.   

### Driving IoT Ecosystems at Scale

The glasses also talk to each other over a mesh network, allowing them 
to scale to large events and concerts, as well as to serve as one 
integral part of IoT ecosystems at home and at work.  We envision the 
insights Captivate glasses can capture about users in these environments 
as a crucial first step towards responsive, immersive environments that 
support user goals.
