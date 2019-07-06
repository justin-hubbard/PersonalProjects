/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class KadenzeChorusFlangerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    KadenzeChorusFlangerAudioProcessorEditor (KadenzeChorusFlangerAudioProcessor&);
    ~KadenzeChorusFlangerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KadenzeChorusFlangerAudioProcessor& processor;

    Slider mDryWetSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    Slider mPhaseOffsetSlider;
    Slider mFeedbackSlider;
    
    ComboBox mType;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KadenzeChorusFlangerAudioProcessorEditor)
};
