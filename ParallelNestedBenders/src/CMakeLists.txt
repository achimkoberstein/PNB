cmake_minimum_required(VERSION 2.8)

add_library(ParallelNestedBenders
	BendersNodeData.cpp
	OsiNeosStochasticSolverInterface.cpp
	OsiPnbStochasticSolverInterface.cpp
	OsiStochasticSolverInterface.cpp
	ParallelNestedBenders.cpp
	PseudoRandomGenerator.cpp
)

