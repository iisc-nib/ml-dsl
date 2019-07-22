#include <iostream>
#include <stdexcept>
#include <string>
#include <strstream>
#include "valuetype.h"
#include "value.h"
#include "neuronproperty.h"
#include "neuron.h"
#include "layer.h"
#include "network.h"

class CollectMergeableNeuronsIntoEnsemblesVisitor : public NetworkVisitor
{
public:
    CollectMergeableNeuronsIntoEnsemblesVisitor()
    { }
    virtual void Visit(Network& network)
    {
        for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
        {
            Layer& layer = network.GetLayer(i);
            layer.AcceptVisitor(*this);
        }
    }
    virtual void Visit(Layer& layer)
    {
        Neuron* currentEnsembleRep = nullptr;
        Ensemble *currentEnsemble = nullptr;

        for (int32_t i=0 ; i<layer.GetNumberOfNeurons() ; ++i)
        {
            Neuron& currentNeuron = layer.GetNeuron(i);
            if (currentEnsembleRep == nullptr || !AreNeuronsMergeable(currentNeuron, *currentEnsembleRep))
            {
                currentEnsemble = &(layer.CreateNewEnsemble());
                currentEnsembleRep = &currentNeuron;
            }
            currentEnsemble->AddNeuron(currentNeuron);
        }
    }
    virtual void Visit(Neuron& neuron)
    {
    }
    virtual void Visit(InputNeuron& inputNeuron)
    {
    }
    virtual void Visit(OutputNeuron& outputNeuron)
    {
    }
};


void CollectMergeableNeuronsIntoEnsembles(Network& network)
{
    CollectMergeableNeuronsIntoEnsemblesVisitor collectVisitor;
    network.AcceptVisitor(collectVisitor);
}

static std::string ConstructLayerOutputName(int32_t layerIdx)
{
    std::strstream strStream;
    strStream << "__layer" << layerIdx << "Result";
    return strStream.str();
}

static VectorType& ConstructLayerOutputType(Layer& layer)
{
    RealType& elemType = *(new RealType());
    VectorType& outputType = *(new VectorType(elemType, layer.GetNumberOfNeurons()));
    return outputType;
}

Function& ConstructIRForNetwork(Network& network)
{
    Layer& inputLayer = network.GetLayer(0);
    VectorType& inputVarType = ConstructLayerOutputType(inputLayer);
    Variable& inputVar = Variable::Create("x", inputVarType);

    Layer& outputLayer = network.GetLayer(network.GetNumberOfLayers()-1);
    VectorType& outputVarType = ConstructLayerOutputType(outputLayer);
    Variable& outputVar = Variable::Create("y", outputVarType);
    
    auto function = Function::Create(inputVar, outputVar);

    // First create a loop to go over the input layer
    // std::string inputLayerOutputVarName = ConstructLayerOutputName(0);
    // VectorType& inputLayerOutputType = ConstructLayerOutputType(inputLayer);
    // Variable& inputLayerOutput = Variable::Create(inputLayerOutputVarName, inputLayerOutputType);
    
    Variable *prevLayerOutput = &inputVar;

    // Loop over the layers
    auto layerLoop = ForLoop::Create(Constant(0), Constant(network.GetNumberOfLayers()));
    for (int32_t i=0 ; i<network.GetNumberOfLayers() ; ++i)
    {
        Layer& layer = network.GetLayer(i);
        std::string layerOutputVarName = ConstructLayerOutputName(i); 
        Variable& layerOutputVar = Variable::Create(layerOutputVarName, ConstructLayerOutputType(layer));
        auto ensembles = layer.GetEnsembles();
        for (int32_t j=0 ; j<ensembles.size() ; ++j)
        {
            auto ensemble = ensembles[j];
            // 0. One loop per ensemble, each loops over all neurons in the ensemble
            auto ensembleLoop = ForLoop::Create(Constant(0), Constant(ensemble.GetNumberOfNeurons()));
            layerLoop.AddStatement(ensembleLoop);

            // 1. Create a ValueSet for all appropriate properties of the neuron (currently assuming its a weighted neuron)
            
            // 2. Construct IR for the representative neuron for the ensemble
            // 3. 

        }

        prevLayerOutput = &layerOutputVar;
    }
}
