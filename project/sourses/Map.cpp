#include "Map.hpp"
#include </home/mariasolovyova/CLionProjects/Evolution/tools/json/single_include/nlohmann/json.hpp>
#include "Pixel.hpp"
#include <iomanip>

using Json = nlohmann::json;

Map::Map(size_t widthCells, size_t heightCells)
{
    widthInCells = widthCells;
    heightInCells = heightCells;
    double deltaX = 9;
    double deltaY = 15;
    double x = deltaX * 6;
    double y = deltaY * 6;
    for (size_t i = 0; i < heightInCells; ++i)
    {
        x = deltaX * 6;
        Row row;
        map.push_back(row);
        if ((i % 2) != 0)
        {
            x += deltaX;
        }
        for (size_t j = 0; j < widthInCells; ++j)
        {
            x += 2 * deltaX;
            map[i].push_back(new Water(x, y, i, j));
        }
        y += deltaY;
    }
    wall = new Wall();
    CreateFood(200);
    SetPoison(200);
}

Map::Map(const Map& mapToCopy)
        :    widthInCells(mapToCopy.widthInCells),
             heightInCells(mapToCopy.heightInCells),
             map(mapToCopy.map),
             organisms(mapToCopy.organisms),
             staticOrganisms(mapToCopy.staticOrganisms),
             evolutionNumber(mapToCopy.evolutionNumber)

{}

Map::Map(Map&& mapToMove)
        :    widthInCells(mapToMove.widthInCells),
             heightInCells(mapToMove.heightInCells),
             map(mapToMove.map),
             organisms(mapToMove.organisms),
             staticOrganisms(std::move(mapToMove.staticOrganisms)),
             evolutionNumber(mapToMove.evolutionNumber)
{}

void Map::MultiplyPixels(int numberOfPixels)
{
    for (int inner = 0; inner < numberOfPixels; ++inner)
    {
        bool flag = false;
        while (!flag)
        {
            size_t xInCells = rand() % (heightInCells - 1);
            size_t yInCells = rand() % (widthInCells - 1);
            if (map[xInCells][yInCells]->GetType() == Hexagon::Type::WATER)
            {
                Pixel* hex = new Pixel(map[xInCells][yInCells]->GetX(), map[xInCells][yInCells]->GetY(), (size_t) xInCells, (size_t) yInCells);
                map[xInCells].erase(yInCells);
                map[xInCells].insert(hex, yInCells);
                organisms.push_back(hex);
                flag = true;
            }
        }
    }
}

void Map::CreateFood(int numberOfFood)
{
    for (int inner = 0; inner < numberOfFood; ++inner)
    {
        bool flag = false;
        while (!flag)
        {
            size_t xInCells = rand() % (heightInCells - 1);
            size_t yInCells = rand() % (widthInCells - 1);
            if (map[xInCells][yInCells]->GetType() == Hexagon::Type::WATER)
            {
                Food* hex = new Food(map[xInCells][yInCells]->GetX(), map[xInCells][yInCells]->GetY(), (size_t)xInCells, (size_t)yInCells);
                map[xInCells].erase(yInCells);
                map[xInCells].insert(hex, yInCells);
                flag = true;
            }
        }
    }
}

void Map::SetPoison(int numberOfPoison)
{
    for (int inner = 0; inner < numberOfPoison; ++inner)
    {
        bool flag = false;
        while (!flag)
        {
            size_t xInCells = rand() % (heightInCells - 1);
            size_t yInCells = rand() % (widthInCells - 1);
            if (map[xInCells][yInCells]->GetType() == Hexagon::Type::WATER)
            {
                Poison* hex = new Poison(map[xInCells][yInCells]->GetX(), map[xInCells][yInCells]->GetY(), (size_t) xInCells, (size_t) yInCells);
                map[xInCells].erase(yInCells);
                map[xInCells].insert(hex, yInCells);
                flag = true;
            }
        }
    }
}

void Map::RecreateMap(const std::vector<Pixel*>& vectorOfNewOrganisms)
{
    Map mapNew;
    mapNew.evolutionNumber = evolutionNumber;
    ClonePixels(mapNew, vectorOfNewOrganisms);
    *this = mapNew;

}

void Map::ClonePixels(Map& mapNew, const std::vector<Pixel*>& vectorOfNewOrganisms)
{
    std::srand(std::time(nullptr));
    mapNew.organisms.clear();
    mapNew.organisms = vectorOfNewOrganisms;
    for (size_t innerOfOrganisms = 0; innerOfOrganisms < vectorOfNewOrganisms.size(); innerOfOrganisms++)
    {
        size_t x = rand() % (heightInCells - 1);
        size_t y = rand() % (widthInCells - 1);
        mapNew.organisms[innerOfOrganisms]->SetLifes(99);
        mapNew.organisms[innerOfOrganisms]->ResetNumberOfLifeIterations();
        mapNew.organisms[innerOfOrganisms]->ResetMedicine();
        mapNew.organisms[innerOfOrganisms]->SetCellStr(x);
        mapNew.organisms[innerOfOrganisms]->SetCellCol(y);
        mapNew.organisms[innerOfOrganisms]->SetX(map[x][y]->GetX());
        mapNew.organisms[innerOfOrganisms]->SetY(map[x][y]->GetY());
        mapNew.SetOrganism(mapNew.organisms[innerOfOrganisms]);
        bool flag = false;
        while (!flag)
        {
            size_t xInCells = rand() % (heightInCells - 1);
            size_t yInCells = rand() % (widthInCells - 1);
            if (mapNew.map[xInCells][yInCells]->GetType() == Hexagon::Type::WATER)
            {
                Brain brain = mapNew.organisms[innerOfOrganisms]->GetBrain();
                brain.Train();
                Pixel* hex = new Pixel(map[xInCells][yInCells]->GetX(), map[xInCells][yInCells]->GetY(), (size_t) xInCells, (size_t) yInCells, brain);
                mapNew.organisms.push_back(hex);
                mapNew.SetOrganism(mapNew.organisms.back());
                flag = true;
            }
        }
    }
}

Map::Map(const std::string& path, int numberOfEvolution)
{
    boost::filesystem::path path_to_file = path + "/" + "Map " + std::to_string(numberOfEvolution);
    if (!boost::filesystem::exists(path_to_file))
        throw std::runtime_error("Error in uploading files");
    *this = Map();
    std::ifstream file(path_to_file.string());
    std::string str;
    str.clear();
    std::string line;
    while (std::getline(file, line))
        str += line;
    file.close();
    Json object = Json::parse(str);
    evolutionNumber = object["Evolution"];
    size_t cellStr;
    size_t cellCol;
    double x;
    double y;
    double medicine;
    double lifes;
    for (auto& s : object["Static Organisms"])
    {
        cellStr = static_cast<size_t>(s["CellStr"]);
        cellCol = static_cast<size_t>(s["CellCol"]);
        x = static_cast<double>(s["X"]);
        y = static_cast<double>(s["Y"]);
        medicine = static_cast<double>(s["Medicine"]);
        lifes = static_cast<double>(s["Lifes"]);
        staticOrganisms.push_back(new Pixel(x, y, cellStr, cellCol, lifes, Brain(s), medicine));
        SetOrganism(staticOrganisms.back());
    }
    for (auto& s : object["Organisms"])
    {
        cellStr = static_cast<size_t>(s["CellStr"]);
        cellCol = static_cast<size_t>(s["CellCol"]);
        x = static_cast<double>(s["X"]);
        y = static_cast<double>(s["Y"]);
        medicine = static_cast<double>(s["Medicine"]);
        lifes = static_cast<double>(s["Lifes"]);
        organisms.push_back(new Pixel(x, y, cellStr, cellCol, lifes, Brain(s), medicine));
        SetOrganism(organisms.back());
    }
    widthInCells = object["Width in Cells"];
    heightInCells = object["Height in Cells"];
}

Map& Map::operator=(const Map& mapOld)
{
    if (&mapOld != this)
    {
        map = mapOld.map;
        organisms = mapOld.organisms;
        staticOrganisms = mapOld.staticOrganisms;
        evolutionNumber = mapOld.evolutionNumber;
        widthInCells = mapOld.widthInCells;
        heightInCells = mapOld.heightInCells;
    }
    return *this;
}

Map& Map::operator=(Map&& mapOld)
{
    map = std::move(mapOld.map);
    organisms = std::move(mapOld.organisms);
    staticOrganisms = std::move(mapOld.staticOrganisms);
    evolutionNumber = mapOld.evolutionNumber;
    widthInCells = mapOld.widthInCells;
    heightInCells = mapOld.heightInCells;
    return *this;
}

void Map::Update()
{
    for (int i = (int)organisms.size() -  1; i >= 0; --i)
    {
        if (organisms[i]->IsAlive())
            organisms[i]->Update(*this);
        else if (!organisms[i]->IsAlive() && organisms.size() > 10)
        {
            map[organisms[i]->GetCellStr()].erase(organisms[i]->GetCellCol());
            if (!organisms[i]->GetisHealfy())
            {
                map[organisms[i]->GetCellStr()].insert(new Poison(organisms[i]->GetX(), organisms[i]->GetY(), organisms[i]->GetCellStr(), organisms[i]->GetCellCol()),
                                                       organisms[i]->GetCellCol());
            }
            else
            {
                map[organisms[i]->GetCellStr()].insert(new Food(organisms[i]->GetX(), organisms[i]->GetY(), organisms[i]->GetCellStr(), organisms[i]->GetCellCol()),
                                                       organisms[i]->GetCellCol());
            }
            organisms.erase(organisms.begin() + i);
        }
        else
        {
            staticOrganisms.push_back(organisms[i]);
            map[organisms[i]->GetCellStr()].erase(organisms[i]->GetCellCol());
            if (!organisms[i]->GetisHealfy())
            {
                map[organisms[i]->GetCellStr()].insert(new Poison(organisms[i]->GetX(), organisms[i]->GetY(), organisms[i]->GetCellStr(), organisms[i]->GetCellCol()),
                                                       organisms[i]->GetCellCol());
            }
            else
            {
                map[organisms[i]->GetCellStr()].insert(new Food(organisms[i]->GetX(), organisms[i]->GetY(), organisms[i]->GetCellStr(), organisms[i]->GetCellCol()),
                                                       organisms[i]->GetCellCol());
            }
            organisms.erase(organisms.begin() + i);
        }
    }
}

Row& Map::operator[](size_t index)
{
    return map[index];
}

const Row& Map::operator[](size_t index) const
{
    return map[index];
}

unsigned int Map::GetWidth() const
{
    return width;
}

unsigned int Map::GetHeight() const
{
    return height;
}

size_t Map::GetWidthInCells() const
{
    return widthInCells;
}

size_t Map::GetHeightInCells() const
{
    return heightInCells;
}

std::vector<Pixel*> Map::GetOrganisms() const
{
    return organisms;
}

std::vector<Pixel*> Map::GetStaticOrganisms() const
{
    return staticOrganisms;
}

size_t Map::GetNumberOfAliveOrganisms() const
{
    return organisms.size();
}

unsigned int Map::GetEvolutionNumber() const
{
    return evolutionNumber;
}

void Map::IncreaseEvolutionNumber()
{
    ++evolutionNumber;
}

int Map::GetTimeToSleep() const
{
    return timeToSleep;
}

void Map::IncreaseTimesToSleep(int x)
{
    timeToSleep += x;
}

void Map::DecreaseTimesToSleep(int x)
{
    timeToSleep -= x;
}

Wall* Map::GetWall() const
{
    return wall;
}

void Map::SetOrganism(Pixel* org)
{
    map[org->GetCellStr()].erase(org->GetCellCol());
    map[org->GetCellStr()].insert(org, org->GetCellCol());
}

void Map::Swap(Hexagon* hex1, Hexagon* hex2)
{
    map[hex1->GetCellStr()][hex1->GetCellCol()] = hex2;
    map[hex2->GetCellStr()][hex2->GetCellCol()] = hex1;
    double copy = hex1->GetX();
    hex1->SetX(hex2->GetX());
    hex2->SetX(copy);
    copy = hex1->GetY();
    hex1->SetY(hex2->GetY());
    hex2->SetY(copy);
    size_t cp = hex1->GetCellStr();
    hex1->SetCellStr(hex2->GetCellStr());
    hex2->SetCellStr(cp);
    cp = hex1->GetCellCol();
    hex1->SetCellCol(hex2->GetCellCol());
    hex2->SetCellCol(cp);
}

void Map::SaveToFile() const
{
    boost::filesystem::path path = boost::filesystem::current_path().parent_path();
    path += "/recordsNew";
    if(!boost::filesystem::exists(path))
        boost::filesystem::create_directory(path);
    std::string path_to_file = path.string() + "/Map " + std::to_string(evolutionNumber);
    std::fstream file(path_to_file, std::ios::app);
    Json j;
    j["Evolution"] = evolutionNumber;
    j["Width in Cells"] = widthInCells;
    j["Height in Cells"] = heightInCells;
    Json jj;
    jj.clear();
    for (auto& s : staticOrganisms)
        jj.push_back(s->getJson());
    j["Static Organisms"] = jj;
    jj.clear();
    for (auto& s : organisms)
        jj.push_back(s->getJson());
    j["Organisms"] = jj;
    file << std::setw(4) << j;
}


void Map::UploadFromFile(int numberOfEvolution)
{
    boost::filesystem::path path = boost::filesystem::current_path().parent_path();
    path += "/recordsNew";
    if(!boost::filesystem::exists(path))
        throw std::runtime_error("UploadFromFile : can't file directory to load from");
    *this = Map(path.string(), numberOfEvolution);
}

void Map::Print(sf::RenderWindow* window) const
{
    window->clear();
    for (size_t i = 1; i < heightInCells; ++i)
    {
        for (size_t j = 0; j < widthInCells; ++j)
        {
            map[i][j]->Print(window);
        }
    }
    sf::Font font;
    font.loadFromFile("../../resourses/Arial.ttf");
    sf::Text text("", font, 25);
    text.setColor(sf::Color::Red);
    text.setString("Number of evolution: " + std::to_string(evolutionNumber));
    text.setPosition(100, 0);
    window->draw(text);
    std::this_thread::sleep_for(std::chrono::milliseconds(GetTimeToSleep()));
}