/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#define MAX_DELAY_TIME 2

//==============================================================================
/**
*/
class KadenzeChorusFlangerAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    KadenzeChorusFlangerAudioProcessor();
    ~KadenzeChorusFlangerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float lin_interp(float inSampleX, float inSampleY, float inFloatPhase);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KadenzeChorusFlangerAudioProcessor)
    
    // Parameter declarations
    
    AudioParameterFloat* mDryWetParameter;
    AudioParameterFloat* mFeedbackParameter;
    AudioParameterFloat* mRateParameter;
    AudioParameterFloat* mDepthParameter;
    AudioParameterFloat* mPhaseOffsetParameter;
    AudioParameterFloat* mTypeParameter;
    
    
    // Circular buffer data
    float mFeedbackLeft;
    float mFeedbackRight;
    
    int mCircularBufferWriteHead;
    int mCircularBufferLength;
    
    float* mCircularBufferLeft;
    float* mCircularBufferRight;
    
    // LFO data
    float mLFOPhase;
};
