#include "PluginEditor.h"
#include <BinaryData.h>

void CCStudioAudioProcessorEditor::GlassLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
    float pos, float start, float end, juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (8.0f);
    const float diameter = juce::jmin (bounds.getWidth(), bounds.getHeight()) - 8.0f;
    auto circle = bounds.withSizeKeepingCentre (diameter, diameter);
    const auto centre = circle.getCentre();
    const float radius = diameter * 0.5f;
    g.setColour (juce::Colour (0x663b1024));
    g.fillEllipse (circle.translated (0, 4));
    g.setColour (juce::Colour (0x99fff1f6));
    g.fillEllipse (circle);
    g.setColour (juce::Colour (0xffb34d70));
    g.drawEllipse (circle, 1.5f);
    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius - 5, radius - 5, 0.0f, start, end, true);
    g.setColour (juce::Colour (0x55a53b60));
    g.strokePath (arc, juce::PathStrokeType (3.0f));
    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - 5, radius - 5, 0.0f, start, start + pos * (end - start), true);
    g.setColour (juce::Colour (0xff8f234b));
    g.strokePath (value, juce::PathStrokeType (3.5f));
    const float angle = start + pos * (end - start);
    juce::Point<float> p (centre.x + std::sin (angle) * (radius - 9), centre.y - std::cos (angle) * (radius - 9));
    g.setColour (juce::Colour (0xff7c173c));
    g.fillEllipse (p.x - 4, p.y - 4, 8, 8);
    g.setColour (juce::Colours::white.withAlpha (0.75f));
    g.fillEllipse (circle.reduced (diameter * 0.28f).translated (-diameter * 0.08f, -diameter * 0.12f));
}

CCStudioAudioProcessorEditor::CCStudioAudioProcessorEditor (CCStudioAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    background = juce::Drawable::createFromImageData (BinaryData::ccstudiobg_svg, BinaryData::ccstudiobg_svgSize);
    title.setText ("CC STUDIO", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, juce::Colour (0xff6f1839));
    title.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title);
    subtitle.setText ("CREATIVE VOCAL REVERB", juce::dontSendNotification);
    subtitle.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xff8b3d5b));
    subtitle.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitle);

    mode.addItemList ({ "DREAM", "VINTAGE", "DARK HALL", "CRYSTAL", "ETHEREAL" }, 1);
    mode.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xdffff5f8));
    mode.setColour (juce::ComboBox::textColourId, juce::Colour (0xff70203f));
    mode.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xffc77b98));
    addAndMakeVisible (mode);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.parameters, "MODE", mode);

    for (int i = 0; i < 10; ++i)
    {
        auto* s = sliders[i];
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        s->setLookAndFeel (&look);
        s->setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xff70203f));
        s->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x99fff5f8));
        s->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (*s);
        labels[i].setText (parameterNames[i], juce::dontSendNotification);
        labels[i].setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        labels[i].setColour (juce::Label::textColourId, juce::Colour (0xff70203f));
        labels[i].setJustificationType (juce::Justification::centred);
        addAndMakeVisible (labels[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, parameterIds[i], *s);
    }
    setSize (1000, 650);
}

void CCStudioAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xffedb8c9));
    if (background != nullptr) background->drawWithin (g, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit, 1.0f);
    g.setColour (juce::Colour (0x55fff8fb));
    g.fillRoundedRectangle (juce::Rectangle<float> (24, 20, (float) getWidth() - 48, (float) getHeight() - 40), 24.0f);
    g.setColour (juce::Colour (0x668b3154));
    g.drawRoundedRectangle (juce::Rectangle<float> (24, 20, (float) getWidth() - 48, (float) getHeight() - 40), 24.0f, 1.2f);
}

void CCStudioAudioProcessorEditor::resized()
{
    title.setBounds (300, 34, 400, 46);
    subtitle.setBounds (350, 78, 300, 18);
    mode.setBounds (380, 112, 240, 34);
    const int left = 70, top = 210, cellW = 172, cellH = 178;
    for (int i = 0; i < 10; ++i)
    {
        const int row = i / 5, col = i % 5;
        const int x = left + col * cellW;
        const int y = top + row * cellH;
        labels[i].setBounds (x, y, 140, 22);
        sliders[i]->setBounds (x + 5, y + 22, 130, 140);
    }
}
