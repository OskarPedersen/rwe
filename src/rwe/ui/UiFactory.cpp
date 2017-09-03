#include "UiFactory.h"

#include <memory>
#include <rwe/ui/UiComponent.h>
#include <rwe/Controller.h>

namespace rwe
{
    UiPanel UiFactory::panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries)
    {
        // first entry sets up the panel
        assert(entries.size() > 0);
        auto panelEntry = entries[0];

        auto texture = textureService->getBitmapRegion(
                background,
                0,
                0,
                panelEntry.common.width,
                panelEntry.common.height);

        UiPanel panel(
            panelEntry.common.xpos,
            panelEntry.common.ypos,
            panelEntry.common.width,
            panelEntry.common.height,
            texture);

        // load panel components
        for (std::size_t i = 1; i < entries.size(); ++i)
        {
            auto& entry = entries[i];

            switch (entry.common.id)
            {
                case GuiElementType::Button:
                {
                    auto stages = entry.stages.value_or(0);
                    std::unique_ptr<UiComponent> btn;
                    if (stages > 1)
                    {
                        btn = std::unique_ptr<UiComponent>(new UiStagedButton(stagedButtonFromGuiFile(name, entry)));
                    }
                    else
                    {
                        btn = std::unique_ptr<UiComponent>(new UiButton(buttonFromGuiFile(name, entry)));
                    }

                    panel.appendChild(std::move(btn));
                    break;
                }
                case GuiElementType::ListBox:
                {
                    std::unique_ptr<UiComponent> elem(new UiListBox(listBoxFromGuiFile(name, entry)));
                    panel.appendChild(std::move(elem));
                    break;
                }
                case GuiElementType::Label:
                {
                    std::unique_ptr <UiComponent> lbl(new UiLabel(labelFromGuiFile(name, entry)));
                    panel.appendChild(std::move(lbl));
                    break;
                }
            }
        }

        return panel;
    }

    UiButton UiFactory::buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry)
    {

        auto graphics = textureService->getGuiTexture(guiName, entry.common.name);
        if (!graphics)
        {
            graphics = getDefaultButtonGraphics(guiName, entry.common.width, entry.common.height);
        }

        auto text = entry.text ? *(entry.text) : std::string("");

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        UiButton button(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            *graphics,
            text,
            font
        );

        auto sound = deduceButtonSound(guiName, entry);

        if (sound)
        {
            button.onClick().subscribe([as = audioService, s = std::move(*sound)](MouseButtonEvent /*event*/){
                as->playSound(s);
            });
        }

        button.onClick().subscribe([c = controller, guiName, name = entry.common.name](MouseButtonEvent /*event*/){
            c->message(guiName, name);
        });

        return button;
    }

    UiFactory::UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs, SkirmishMenuModel* model, Controller* controller)
        : textureService(textureService), audioService(audioService), soundLookup(soundLookup), vfs(vfs), model(model), controller(controller)
    {}

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultButtonGraphics(const std::string& guiName, int width, int height)
    {
        // hack for SINGLE.GUI buttons
        if (width == 118 && height == 18)
        {
            width = 120;
            height = 20;
        }

        auto sprites = textureService->getGuiTexture(guiName, "BUTTONS0");
        if (sprites)
        {
            auto it = std::find_if(
                (*sprites)->sprites.begin(),
                (*sprites)->sprites.end(),
                [width, height](const Sprite& s) {
                    return s.bounds.width() == width && s.bounds.height() == height;
                }
            );

            if (it != (*sprites)->sprites.end())
            {
                auto spritesView = std::make_shared<SpriteSeries>();
                spritesView->sprites.push_back(*(it++));
                assert(it != (*sprites)->sprites.end());
                spritesView->sprites.push_back(*(it++));
                return spritesView;
            }
        }

        // default behaviour
        auto texture = textureService->getDefaultTexture();
        Sprite sprite(Rectangle2f::fromTopLeft(0.0f, 0.0f, width, height), texture);
        auto series = std::make_shared<SpriteSeries>();
        series->sprites.push_back(sprite);
        series->sprites.push_back(sprite);
        return series;
    }

    boost::optional<AudioService::SoundHandle> UiFactory::getButtonSound(const std::string& buttonName)
    {
        auto soundBlock = soundLookup->findBlock(buttonName);
        if (!soundBlock)
        {
            return boost::none;
        }

        auto soundName = soundBlock->findValue("sound");
        if (!soundName)
        {
            return boost::none;
        }

        return audioService->loadSound(*soundName);
    }

    UiLabel UiFactory::labelFromGuiFile(const std::string& guiName, const GuiEntry& entry)
    {
        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        UiLabel label(
                entry.common.xpos,
                entry.common.ypos,
                entry.common.width,
                entry.common.height,
                entry.text.value_or(""),
                font
        );

        if (guiName == "SELMAP" && entry.common.name == "DESCRIPTION")
        {
            model->selectedMap.subscribe([&label](const auto& selectedMap) {
                if (selectedMap)
                {
                    label.setText(selectedMap->description);
                }
                else
                {
                    label.setText("");
                }
            });
        }

        return label;
    }

    UiStagedButton UiFactory::stagedButtonFromGuiFile(const std::string& guiName, const GuiEntry& entry)
    {
        auto graphics = textureService->getGuiTexture(guiName, entry.common.name);
        if (!graphics)
        {
            graphics = getDefaultStagedButtonGraphics(guiName, entry.stages.get());
        }

        auto labels = entry.text ? utf8Split(entry.text.get(), '|') : std::vector<std::string>();

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        UiStagedButton button(
                entry.common.xpos,
                entry.common.ypos,
                entry.common.width,
                entry.common.height,
                *graphics,
                labels,
                font
        );

        auto sound = deduceButtonSound(guiName, entry);

        if (sound)
        {
            button.onClick([as = audioService, s = std::move(*sound)](MouseButtonEvent /*event*/){
                as->playSound(s);
            });
        }

        button.onClick([c = controller, guiName, name = entry.common.name](MouseButtonEvent /*event*/){
            c->message(guiName, name);
        });

        return button;
    }

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultStagedButtonGraphics(const std::string& guiName, int stages)
    {
        assert(stages >= 2 && stages <= 4);
        std::string entryName("stagebuttn");
        entryName.append(std::to_string(stages));

        auto sprites = textureService->getGuiTexture(guiName, entryName);
        if (sprites)
        {
            return *sprites;
        }

        // default behaviour
        auto texture = textureService->getDefaultTexture();
        Sprite sprite(Rectangle2f::fromTopLeft(0.0f, 0.0f, 120.0f, 20.0f), texture);
        auto series = std::make_shared<SpriteSeries>();
        series->sprites.push_back(sprite);
        series->sprites.push_back(sprite);
        return series;
    }

    UiListBox UiFactory::listBoxFromGuiFile(const std::string& guiName, const GuiEntry& entry)
    {
        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        UiListBox listBox(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            font
        );

        if (entry.common.name == "MAPNAMES")
        {
            auto mapNames = vfs->getFileNames("maps", ".ota");

            for (const auto& e : mapNames)
            {
                // chop off the extension while adding
                listBox.appendItem(e.substr(0, e.size() - 4));
            }

            model->selectedMap.subscribe([&listBox](const auto& selectedMap) {
                if (selectedMap)
                {
                    listBox.setSelectedItem(selectedMap->name);
                }
                else
                {
                    listBox.clearSelectedItem();
                }
            });

            listBox.selectedIndex().subscribe([&listBox, c = controller](const auto& selectedMap) {
                if (selectedMap)
                {
                    c->setSelectedMap(listBox.getItems()[*selectedMap]);
                }
                else
                {
                    c->clearSelectedMap();
                }
            });
        }

        return listBox;
    }

    boost::optional<AudioService::SoundHandle> UiFactory::deduceButtonSound(const std::string& guiName, const GuiEntry& entry)
    {
        auto sound = getButtonSound(entry.common.name);
        if (!sound && (entry.common.name == "PrevMenu" || entry.common.name == "PREVMENU"))
        {
            sound = getButtonSound("PREVIOUS");
        }
        if (!sound)
        {
            sound = getButtonSound(guiName);
        }
        if (!sound && guiName == "SELMAP")
        {
            sound = getButtonSound("SMALLBUTTON");
        }
        if (!sound && entry.common.width == 96 && entry.common.height == 20)
        {
            sound = getButtonSound("BIGBUTTON");
        }

        return sound;
    }
}
