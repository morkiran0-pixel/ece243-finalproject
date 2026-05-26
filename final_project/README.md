# Final Project — Neural Network on FPGA

**Course:** ECE243 — University of Toronto  

## Project Overview
Designed and implemented a feedforward neural network in C and deployed it on the DE1-SoC FPGA board to classify urinary tract infection (UTI) cases using patient health data.

The system supports configurable network architecture and training parameters through hardware controls and provides real-time VGA visualization of training behaviour.

## Demo

### Real-Time Neural Network Visualization

![FPGA Neural Network Demo](screenshots/vga_training_visualization.png)

The FPGA visualization displays:
- Neural network topology
- Training progress
- Accuracy indicator
- Configurable hidden layers and epochs

## Dataset
- 119 patient records
- 6 input features:
  - Temperature
  - Nausea
  - Lumbar pain
  - Urine pushing
  - Micturition pain
  - Burning sensation
- 2 output classes

## Features
- Forward propagation using sigmoid activation
- Backpropagation with configurable learning rate
- Real-time VGA visualization
- Hardware interaction through switches and pushbuttons
- Live inference after training

## Technologies
- C
- FPGA (DE1-SoC)
- Embedded Systems
- Neural Networks
- VGA Interface

## Repository Structure

final_project/
├── neural_network.c
├── README.md
└── screenshots/
    └── vga_training_visualization.png

## Learning Outcomes
- Implemented neural network training without external ML libraries
- Connected software algorithms with FPGA hardware interaction
- Built visualization tools for monitoring training behaviour
