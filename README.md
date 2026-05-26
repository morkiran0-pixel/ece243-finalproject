
## Final Project — Neural Network on FPGA
A feedforward neural network implemented in C and deployed on the DE1-SoC FPGA board to classify urinary tract infection diagnoses from patient data.

**Dataset:** 119 patient records, 6 input features (temperature, nausea, lumbar pain, urine pushing, micturition pain, burning sensation), 2 output classes  
**Architecture:** Configurable hidden layers and epochs via hardware switches and pushbuttons  
**Key features:**
- Forward pass with sigmoid activation
- Backpropagation with configurable learning rate
- Real-time VGA visualization of the network topology, training accuracy bar, and epoch progress
- Hardware I/O: switches set layer count and epochs, pushbuttons confirm inputs
- Live accuracy display and per-sample inference mode after training

---

## Tools & Technologies
- **Verilog HDL** — processor design and simulation
- **Intel Quartus Prime** — synthesis and FPGA deployment
- **C** — embedded programming on ARM Cortex-A9
- **DE1-SoC FPGA Board** — Cyclone V, VGA output, memory-mapped I/O
