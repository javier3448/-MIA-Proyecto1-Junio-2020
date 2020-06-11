#include "Disk/diskmanager.h"
#include <cstring>
#include "raidonefile.h"
#include "mystringutil.h"
#include "FileSystem/extmanager.h"

using std::string;

//<std::string, std::pair<std::string, std::string> es uno de los peor chapuz de todos los tiempos
std::unordered_map<std::string, MountedPart> mountedPartitions = std::unordered_map<std::string, MountedPart>();

char ZERO = 0;//Deberian ser const pero no me deja pasarlas a RaidOneFile si son const
char ONE = 1;

int DiskManager::toBytes(int size, char unit)//Deberia estar en un archivo separado, algo asi como diskUtility o byteUtility
{
    if(unit == 'k')
        return size * 1024;
    else if(unit == 'm')
        return size = size * 1024 * 1024;
    else //unit == 'b'
        return size;
}

int DiskManager::mkDisk(const string& path, int size, char fit, char unit)
{
    RaidOneFile file(path, std::ios::binary | std::ios::out);

    //TODO: Revisar que esto sea seguro de correr con sudo y que no elimine nada por accidente
    //TODO: Poner esto en los rep tambien
    //CHAPUZ HORRIBLE para que cree el directorio de ser necesario
    if(!file.areOpen())
    {
        std::string dirPath = path.substr(0, path.find_last_of('/'));
        int status = system((std::string("mkdir -p '") + dirPath + "'").c_str());
        if(status != 0){
            Consola::reportarError("Direccion: " + path + " no es valida. " + std::strerror(errno));
            return -1;
        }
        file.open(path, std::ios::binary | std::ios::out);
    }

    //Queremos que size represente el numero de bytes
    size = toBytes(size, unit);

    //Crea un nuevo mbr
    Mbr mbr(size, fit);

    //Escribe los datos del mbr en el disko
    file.write((char*)&mbr, sizeof(Mbr));

    //Llenan de 0 los discos
    file.seekp(size - 1);

    file.write(&ZERO, 1);

    file.close();
    return 0;
}

//retorna -1 si no logro elimnar
int DiskManager::rmDisk(const string& path)
{
    int result = 0;

    if(std::remove(path.c_str()) != 0){
        Consola::reportarError("No se pudo eliminar el disco en direccion: " + path);
        result = -1;
    }

    if(std::remove(MyStringUtil::appendBeforeExtension(path, "raid").c_str()) != 0){
        Consola::reportarError("No se pudo eliminar el disco en direccion: " + path);
        result = -1;
    }

    return  result;
}

//freeIntervals debe tener tamanno PRIMARY_COUNT + 1. sortedPartition debe de ser de tamanno PRIMARY_COUNT
//freeIntervals ya tiene el primer intervalo declarado pero los demas no
int getPrimaryPartitionStartFirstFit(Mbr* mbr, Partition* sortedPartition[], std::pair<int, int> firstFreeInterval, int size)
{
    if (size <= firstFreeInterval.second - firstFreeInterval.first)
        return firstFreeInterval.first;

    //Sigue buscando
    int i;
    int start;
    int end;
    for (i = 1; i < PART_COUNT + 1; i++)
    {
        start = sortedPartition[i-1]->start + sortedPartition[i-1]->size;
        if (sortedPartition[i]->isNull())
        {
            end = mbr->size;
            i = PRIMARY_COUNT + 1;//Chapus para que ya no continue con el siglo despues de terminar la iteracion actual
        }
        else
        {
            end= sortedPartition[i]->start;
        }

        //Como es first fit lo trat de meter de una vez al disco
        if (size <= end - start)
            return start;
    }

    Consola::reportarError("No se ha podido crear la particion con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible");
    return -1;
}

int getPrimaryPartitionStart(Mbr* mbr, int size, char fit)
{
    Partition* sortedPartition[PART_COUNT + 1];//Particiones ordenadas por partStart
    int i;
    for (i = 0; i < PRIMARY_COUNT; i++) {
        sortedPartition[i] = &(mbr->primaryPartition[i]);
    }
    sortedPartition[i] = &(mbr->extendedPartition);
    i++;
    //La ultima posicion del sortedPartition es un null Partition para que el caso general general abarque el caso del ultimo espacio disponible en el disco
    //Para tener un null terminated array
    Partition nullPart;
    sortedPartition[i] = &(nullPart);

    std::sort(sortedPartition, sortedPartition + i, [](Partition* a, Partition* b)
    {
        if(a->isNull())
            return false;
        if(b->isNull())
            return true;
        return a->start < b->start;
    });

    std::pair<int, int> freeIntervals[PART_COUNT + 1];//intervalos desocupados en el disco

    //determina el primer espacio disponible (de sizeof mbr a la primera particion)
    freeIntervals[0].first = sizeof(Mbr);
    if (sortedPartition[0]->isNull()) //Caso especial si no se a creado ninguna particion
    {
        freeIntervals[0].second = mbr->size;
        if (size <= freeIntervals[0].second - freeIntervals[0].first)
        {
            return freeIntervals->first;
        }
        else
        {
            Consola::reportarError("No se ha podido crear la particion con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible");
            return -1;
        }
    }
    else
    {
        freeIntervals[0].second = sortedPartition[0]->start;
    }

    if(fit == 'f')
        return getPrimaryPartitionStartFirstFit(mbr, sortedPartition, freeIntervals[0], size);

    for (i = 1; i < PART_COUNT + 1; i++)
    {
        freeIntervals[i].first = sortedPartition[i-1]->start + sortedPartition[i-1]->size;
        if (sortedPartition[i]->isNull())
        {
            freeIntervals[i].second = mbr->size;
            i = PRIMARY_COUNT + 1;//Chapus para que ya no continue con el siglo despues de terminar la iteracion actual
        }
        else
        {
            freeIntervals[i].second = sortedPartition[i]->start;
        }
    }

    //El for siempre va a buscar el minimo, para evitarnos hacer dos for si es best fit se multiplica comparisonResult por -1 para que
    //el for pueda buscar siempre el minimo
    int factor = ( fit == 'b' ? 1 : -1 );
    std::pair<int, int>* correctFit = nullptr;
    int currentMin = mbr->size + 1;
    int comparisonResult;

    for (i = 0; i < PART_COUNT + 1; i++)
    {
        if (size <= freeIntervals[i].second - freeIntervals[i].first)
        {
            comparisonResult = factor * freeIntervals[i].second - (freeIntervals[i].first + size);
            if (comparisonResult < currentMin)
            {
                correctFit = &freeIntervals[i];
                currentMin = comparisonResult;
            }
        }
    }

    if(correctFit)
    {
        return correctFit->first;
    }
    else
    {
        Consola::reportarError("No se ha podido crear la particion con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible");
        return -1;
    }
}

int fDiskNewPrimary(RaidOneFile* file, Mbr* mbr, const string &name, int size, char fit)
{
    Partition* principal = nullptr;
    for (int i = 0; i < PRIMARY_COUNT; i++)
    {
        if (mbr->primaryPartition[i].isNull()) {
            principal = &(mbr->primaryPartition[i]);
            break;
        }
    }
    if(!principal)
    {
        Consola::reportarError("Disco solo puede tener 3 particiones principales");
        return -1;
    }

    //int start = getPrimaryPartitionStart(mbr, size, fit);
    int start = getPrimaryPartitionStart(mbr, size, mbr->diskFit);//Parche para que meta la particion con el fit del disco y en el objeto partition ponga la particion que le mandaron como parametro

    if(start == -1)
        return start;

    //Actualizar el mbr en memoria
    principal->Init('p', fit, start, size, name);

    //Actualiza el mbr en el archivo binario
    file->seekp(0, std::ios::beg);
    file->write((char*)mbr, sizeof(Mbr));

    return start;
}

void initTopEbr(Ebr* ebr, int start)
{
    ebr->start = start;
    ebr->size = sizeof (Ebr);
    ebr->next = -1;
    ebr->status = 0;
    ebr->fit = 0;
    strcpy(ebr->name, "$TopEbr");
}

int fDiskNewExtended(RaidOneFile* file, Mbr* mbr, const string &name, int size, char fit)
{
    if(!mbr->extendedPartition.isNull())
    {
        Consola::reportarError("Disco solo puede tener 1 particion extendida");
        return -1;
    }

    //int start = getPrimaryPartitionStart(mbr, size, fit);
    int start = getPrimaryPartitionStart(mbr, size, mbr->diskFit);//Parche para que meta la particion con el fit del disco y en el objeto partition ponga la particion que le mandaron como parametro

    if(start == -1)
        return start;

    //Actualizar el mbr en memoria
    mbr->extendedPartition.Init('e', fit, start, size, name);

    //Actualiza el mbr en el archivo binario
    file->seekg(0, std::ios::beg);
    file->write((char*)mbr, sizeof(Mbr));

    //Crea un TopEbr y lo agrega al archivo:
    Ebr ebr;
    initTopEbr(&ebr, start);
    file->seekg(start, std::ios::beg);
    file->write((char*)&ebr, sizeof(Ebr));

    return start;
}

int getLogicPartitionStartFirstFit(Partition* extendedPartition,const std::vector<Ebr> &ebrs, const std::pair<int, int> &firstFreeInterval, int size)
{
    if (size <= firstFreeInterval.second - firstFreeInterval.first)
        return firstFreeInterval.first;

    //Sigue buscando
    unsigned long i;
    int start;
    int end;
    for (i = 2; i < ebrs.size(); i++)
    {
        start = ebrs[i - 1].start + ebrs[i - 1].size;
        if (ebrs[i].isNull())
        {
            end = extendedPartition->start + extendedPartition->size;
            i = ebrs.size();//Chapus para que ya no continue con el siglo despues de terminar la iteracion actual
        }
        else
        {
            end = ebrs[i].start;
        }

        if (size <= end - start)
            return start;
    }

    Consola::reportarError("No se ha podido crear la particion con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible");
    return -1;
}

int getLogicPartitionStart(Partition* extendedPartition, const std::vector<Ebr>& ebrs, int size, char fit)
{
    std::vector<std::pair<int, int>> freeIntervals(ebrs.size() - 1);

    //determina el primer espacio disponible (de sizeof ebr(topEbr) a la primera particion logica)
    freeIntervals[0].first = extendedPartition->start + (int)sizeof(Ebr);
    if (ebrs[1].isNull()) //Caso especial si no se a creado ninguna particion logica
    {
        freeIntervals[0].second = extendedPartition->start + extendedPartition->size;
        if (size <= freeIntervals[0].second - freeIntervals[0].first)
        {
            return freeIntervals[0].first;
        }
        else
        {
            Consola::reportarError("No se ha podido crear la particion logica con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible en la particion extendida");
            return -1;
        }
    }
    else
    {
        freeIntervals[0].second = ebrs[1].start;
    }

    if(fit == 'f')
        return getLogicPartitionStartFirstFit(extendedPartition, ebrs, freeIntervals[0], size);

    unsigned long i;
    for (i = 2; i < ebrs.size(); i++)
    {
        freeIntervals[i - 1].first = ebrs[i - 1].start + ebrs[i - 1].size;
        if (ebrs[i].isNull())
        {
            freeIntervals[i - 1].second = extendedPartition->start + extendedPartition->size;
            i = ebrs.size();//Chapus para que ya no continue con el siglo despues de terminar la iteracion actual
        }
        else
        {
            freeIntervals[i - 1].second = ebrs[i].start;
        }
    }

    //El for siempre va a buscar el minimo, para evitarnos hacer dos for si es best fit se multiplica comparisonResult por -1 para que
    int factor = ( fit == 'w' ? 1 : -1 );//para que el for pueda buscar siempre el minimo:
    std::pair<int, int>* correctFit = nullptr;
    int currentMin = extendedPartition->size;
    int comparisonResult;

    for (i = 0; i < freeIntervals.size(); i++)
    {
        if (size <= freeIntervals[i].second - freeIntervals[i].first)
        {
            comparisonResult = factor * freeIntervals[i].second - (freeIntervals[i].first + size);
            if (comparisonResult < currentMin)
            {
                correctFit = &freeIntervals[i];
                currentMin = comparisonResult;
            }
        }
    }

    if(correctFit)
    {
        return correctFit->first;
    }
    else
    {
        Consola::reportarError("No se ha podido crear la particion logica con tamanno: " + std::to_string(size) + ". No hay suficiente espacio disponible en la particion extendida");
        return -1;
    }
}

int fDiskNewLogic(RaidOneFile* file, Mbr* mbr, const string &name, int size, char fit)
{
    if (mbr->extendedPartition.isNull())
    {
        Consola::reportarError("No se puede crear una particion logica sin crear antes una particion extendida");
        return -1;
    }

    Partition* extendedPartition = &(mbr->extendedPartition);

    std::vector<Ebr> ebrs;
    ebrs.reserve(16);

    //Consigue el TopEbr
    Ebr lastEbr;
    file->seekg(extendedPartition->start, std::ios::beg);
    file->read((char*)&lastEbr, sizeof(Ebr));
    ebrs.push_back(lastEbr);

    while(lastEbr.next != -1)
    {
        file->seekg(lastEbr.next, std::ios::beg);
        file->read((char*)&lastEbr, sizeof(Ebr));
        ebrs.push_back(lastEbr);
    }//Para cuando termine este ciclo lastEbr va ser (en terminos de la lista enlazada de ebrs en el disco, el ebr previo al nuevoEbr)

    //se agrega 1 mas a ebrs para que sea null terminated
    ebrs.push_back(Ebr());

    std::sort(ebrs.begin(), ebrs.end(), [](Ebr a, Ebr b)//A diferencia de
        {
            if(a.isNull())
                return false;
            if(b.isNull())
                return true;
            return a.start < b.start;
        });
    
    //int start = getLogicPartitionStart(extendedPartition, ebrs, size, fit);
    int start = getLogicPartitionStart(extendedPartition, ebrs, size, mbr->diskFit);//Parche para que meta la particion con el fit del disco y en el objeto partition ponga la particion que le mandaron como parametro

    if(start == -1)
        return start;

    //Actualizar el lastEbr en memoria
    lastEbr.next = start;

    //Actualiza el lastEbr en disco
    file->seekp(lastEbr.start, std::ios::beg);
    file->write((char*)&lastEbr, sizeof(Ebr));

    //Crea al nuevo ebr
    Ebr newEbr(start, size, -1, '\0', fit, name);

    //Lo agrega al disco
    file->seekp(start, std::ios::beg);
    file->write((char*)&newEbr, sizeof(Ebr));

    return start;
}

bool isPartitionNameUnique(RaidOneFile* file, Mbr* mbr, const std::string& name){
    for(int i = 0; i < PART_COUNT; i++)
    {
        if(name == mbr->primaryPartition[i].name){
            return false;
        }
    }
    if(mbr->extendedPartition.isNull()){
        return true;
    }

    Ebr currEbr;
    file->seekg(mbr->extendedPartition.start, std::ios::beg);
    file->read((char*)&currEbr, sizeof(Ebr));

    while(true)
    {
        if(name == currEbr.name){
            return false;
        }
        if(currEbr.next == -1){
            break;
        }
        file->seekg(currEbr.next, std::ios::beg);
        file->read((char*)&currEbr, sizeof(Ebr));
    }

    return true;
}

int DiskManager::fDiskNew(const string& path, const string& name, int size, char fit, char unit, char type)
{
    //Conseguir mbr
    RaidOneFile file(path, std::ios::binary | std::ios::out | std::ios::in);

    if(!file.areOpen())
    {
        Consola::reportarError("Direccion: " + path + " no es valida");
        return -1;
    }

    size = toBytes(size, unit);

    Mbr mbr;
    file.read((char*)&mbr, sizeof(Mbr));//Si lee un archivo no valido, pasa comportamiento no definido

    if(!isPartitionNameUnique(&file, &mbr, name)){
        Consola::reportarError("Ya existe una particion con nombre: " + name);
        return -1;
    }

    int result = 0;

    if(type == 'p')
        result = fDiskNewPrimary(&file, &mbr, name, size, fit);
    else if(type == 'e')
        result = fDiskNewExtended(&file, &mbr, name, size, fit);
    else //type == l
        result = fDiskNewLogic(&file, &mbr, name, size, fit);

    file.close();
    return result;
}

int fDiskDeletePrimary(RaidOneFile* file, Mbr* mbr, Partition* p, char deleteType)
{
    if(deleteType == 'f'){//full
        file->seekp(p->start, std::ios::beg);
        std::vector<char> zeros(p->size);
        fill(zeros.begin(), zeros.end(), 0);
        file->write(&zeros[0], p->size);
    }
    *p = Partition();//pasa a ser null el valor de la particion

    //Sobreescribe el mbr
    file->seekp(0, std::ios::beg);
    file->write((char*)mbr, sizeof(Mbr));

    return 0;
}

//currEbr es el que se va a eliminar
int fDiskDeleteLogic(RaidOneFile* file, Ebr* prevEbr, Ebr* currEbr, char deleteType)
{
    if(deleteType == 'f'){//full
        file->seekp(currEbr->start, std::ios::beg);
        std::vector<char> zeros(currEbr->size);
        fill(zeros.begin(), zeros.end(), 0);
        file->write(&zeros[0], currEbr->size);
    }
    //Actualiza el next del prevEbr
    prevEbr->next = currEbr->next;

    //Sobreescribe el prevEbr
    file->seekp(prevEbr->start, std::ios::beg);
    file->write((char*)prevEbr, sizeof(Ebr));

    return 0;
}



//TODO TODO TODO: PROBAR ESTA MIERDA!
//FALTA PROBARLO
int DiskManager::fDiskDelete(const std::string& path, const std::string& name, char deleteType)
{
    //Conseguir que particion hay que remover
    RaidOneFile file(path, std::ios::binary | std::ios::out | std::ios::in);
    if(!file.areOpen())
    {
        Consola::reportarError("Direccion: " + path + " no es valida");
        return -1;
    }
    Mbr mbr;
    file.read((char*)&mbr, sizeof(Mbr));

    int result = -1;
    bool isPrimary = false;
    bool isLogic = false;

    //Revisar si es primaria:
    Partition* p = mbr.primaryPartition;
    int i;
    for(i = 0; i < PRIMARY_COUNT + 1; i++)
    {
        if(strcmp(p->name, name.c_str()) == 0)
        {
            result = fDiskDeletePrimary(&file, &mbr, p, deleteType);
            isPrimary = true;
            break;
        }
        p++;
    }
    p = nullptr;//Solo por si acaso porque p puede tener una direccion no valida despues del for

    //Busca entre las logica si no encontro una con el mismo nombre en las primarias y existen una particion extendida
    if(!isPrimary && !mbr.extendedPartition.isNull())
    {
        Ebr prevEbr;//Ebr previo al que se va a eliminar
        Ebr currEbr;//Ebr que se va a eliminar
        file.seekg(mbr.extendedPartition.start, std::ios::beg);
        file.read((char*)&prevEbr, sizeof(Ebr));//currEbr seria igual a topEbr y preEbr seria null por ahora
        if(prevEbr.next != -1){//Si no es verdadero es porque no se tiene ninguna particion logica
            file.seekg(prevEbr.next, std::ios::beg);
            file.read((char*)&currEbr, sizeof(Ebr));
            while(true)//Se supone que la primera corrida de este while nunca va a dar verdadero porque va usar al topEbr como curr
            {
                if(strcmp(currEbr.name, name.c_str()) == 0)
                {
                    result = fDiskDeleteLogic(&file, &prevEbr, &currEbr, deleteType);
                    isLogic = true;
                    break;
                }
                if(currEbr.next < 0){
                    break;
                }
                prevEbr = currEbr;
                file.seekg(currEbr.next, std::ios::beg);
                file.read((char*)&currEbr, sizeof(Ebr));
            }
        }

    }

    file.close();

    if(!isPrimary && !isLogic)
    {
        Consola::reportarError("No existe ninguna particion con el nombre: " + name);
        result = -1;
    }

    return result;
}

std::vector<std::pair<int, int>> getFreeIntervalsPrimaryPartition(Mbr* mbr)
{
    Partition* sortedPartition[PART_COUNT + 1];//Particiones ordenadas por partStart
    unsigned int i;
    for (i = 0; i < PRIMARY_COUNT; i++) {
        sortedPartition[i] = &(mbr->primaryPartition[i]);
    }
    sortedPartition[i] = &(mbr->extendedPartition);
    i++;
    //La ultima posicion del sortedPartition es un null Partition para que el caso general general abarque el caso del ultimo espacio disponible en el disco
    //Para tener un null terminated array
    Partition nullPart;
    sortedPartition[i] = &(nullPart);

    std::sort(sortedPartition, sortedPartition + i, [](Partition* a, Partition* b)
    {
        if(a->isNull())
            return false;
        if(b->isNull())
            return true;
        return a->start < b->start;
    });

    std::vector<std::pair<int, int>> freeIntervals;
    freeIntervals.reserve(PART_COUNT + 1);//intervalos desocupados en el disco

    std::pair<int, int> newPairEntry;

    //determina el primer espacio disponible (de sizeof mbr a la primera particion)
    newPairEntry.first = sizeof(Mbr);
    if (sortedPartition[0]->isNull()) //Caso especial si no se a creado ninguna particion
    {
        newPairEntry.second = mbr->size;
        return freeIntervals;
    }
    else
    {
        newPairEntry.second = sortedPartition[0]->start;
    }
    freeIntervals.push_back(newPairEntry);

    for (i = 1; i < PART_COUNT + 1; i++)
    {
        newPairEntry.first = sortedPartition[i-1]->start + sortedPartition[i-1]->size;
        if (sortedPartition[i]->isNull())
        {
            newPairEntry.second = mbr->size;
            i = PRIMARY_COUNT + 1;//Chapus para que ya no continue con el siglo despues de terminar la iteracion actual
        }
        else
        {
            newPairEntry.second = sortedPartition[i]->start;
        }
        freeIntervals.push_back(newPairEntry);
    }

    return freeIntervals;
}

int fDiskAddToPrimary(RaidOneFile* file, Mbr* mbr, Partition* p, int size)
{
    if(size < 1)
    {
        if(p->size + size < 1){
            Consola::reportarError("No se puede aplicar add->" + std::to_string(size) + " a una particion de tamanno: " + std::to_string(p->size));
            return -1;
        }

        p->size += size;

        file->seekp(0, std::ios::beg);
        file->write((char*)mbr, sizeof(Mbr));

        return 0;
    }
    else
    {
        std::vector<std::pair<int, int>> freeIntervals = getFreeIntervalsPrimaryPartition(mbr);

        std::pair<int, int>* leftFreeInterval = nullptr;
        std::pair<int, int>* rightFreeInterval = nullptr;

        for(auto interval : freeIntervals){
            if(interval.second == p->start)
            {
                leftFreeInterval = &interval;
            }
            if(interval.first == (p->start + p->size))
            {
                rightFreeInterval = &interval;
                break;
            }
        }

        //solo agrega a la derecha de la particion
        int freeSpace = (rightFreeInterval->second - rightFreeInterval->first);
        if(freeSpace < size){
            Consola::reportarError("No se puede aplicar add->" + std::to_string(size) + " a una particion de tamanno: " + std::to_string(p->size) + " solo se dispone de: " + std::to_string(freeSpace));
            return -1;
        }


    }

    p->size += size;

    file->seekp(0, std::ios::beg);
    file->write((char*)mbr, sizeof(Mbr));

    return 0;

}

int fDiskAddToLogic(RaidOneFile* file, const Partition& extendedPart,  Ebr* ebr, Ebr* nextEbr, int size)
{
    if(size < 1)
    {
        if(ebr->size + size <= sizeof(Ebr)){
            Consola::reportarError("No se puede aplicar add->" + std::to_string(size) + " a una particion de tamanno: " + std::to_string(ebr->size));
            return -1;
        }

        ebr->size += size;

        file->seekp(0, std::ios::beg);
        file->write((char*)ebr, sizeof(Ebr));

        return 0;
    }
    else
    {
        int freeIntervalBeg = ebr->start + ebr->size;
        int freeIntervalEnd = (nextEbr->isNull() ? extendedPart.start + extendedPart.size : nextEbr->start);
        //solo agrega a la derecha de la particion
        int freeSpace = (freeIntervalEnd - freeIntervalBeg);
        if(freeSpace < size){
            Consola::reportarError("No se puede aplicar add->" + std::to_string(size) + " a una particion de tamanno: " + std::to_string(ebr->size) + " solo se dispone de: " + std::to_string(freeSpace));
            return -1;
        }

        ebr->size += size;

        file->seekp(0, std::ios::beg);
        file->write((char*)ebr, sizeof(Ebr));

        return 0;
    }

}

//El nombre size del 3er parametro es algo confuso porque en lwh el nombre de la cantidad de bytes a agregar es add
int DiskManager::fDiskAdd(const std::string& path, const std::string& name, int size, char unit)
{
    //Conseguir que particion hay que remover
    RaidOneFile file(path, std::ios::binary | std::ios::out | std::ios::in);
    if(!file.areOpen())
    {
        Consola::reportarError("Direccion: " + path + " no es valida");
        return -1;
    }
    Mbr mbr;
    file.read((char*)&mbr, sizeof(Mbr));

    size = toBytes(size, unit);

    int result = -1;
    bool isPrimary = false;
    bool isLogic = false;

    //Revisar si es primaria:
    Partition* p = mbr.primaryPartition;
    int i;
    for(i = 0; i < PRIMARY_COUNT + 1; i++)
    {
        if(strcmp(p->name, name.c_str()) == 0)//TODO: cambiar a comparacion de strings con c++
        {
            result = fDiskAddToPrimary(&file, &mbr, p, size);
            isPrimary = true;
            break;
        }
        p++;
    }
    p = nullptr;//Solo por si acaso porque p puede tener una direccion no valida despues del for

    //Busca entre las logica si no encontro una con el mismo nombre en las primarias y existen una particion extendida
    if(!isPrimary && !mbr.extendedPartition.isNull())
    {
        std::vector<Ebr> ebrs;
        ebrs.reserve(16);

        Ebr lastEbr;
        file.seekg(mbr.extendedPartition.start, std::ios::beg);
        file.read((char*)&lastEbr, sizeof(Ebr));
        ebrs.push_back(lastEbr);

        while(lastEbr.next != -1)
        {
            file.seekg(lastEbr.next, std::ios::beg);
            file.read((char*)&lastEbr, sizeof(Ebr));
            ebrs.push_back(lastEbr);
        }//Para cuando termine este ciclo lastEbr va ser (en terminos de la lista enlazada de ebrs en el disco, el ebr previo al nuevoEbr)

        //se agrega 1 mas a ebrs para que sea null terminated
        ebrs.push_back(Ebr());

        std::sort(ebrs.begin(), ebrs.end(), [](Ebr a, Ebr b)//A diferencia de
            {
                if(a.isNull())
                    return false;
                if(b.isNull())
                    return true;
                return a.start < b.start;
            });

        for (int i = 0; i < ebrs.size() - 1; i++) {
            if(ebrs[i].name == name){
                result = fDiskAddToLogic(&file, mbr.extendedPartition, &ebrs[i], &ebrs[i + 1], size);
                isLogic = true;
                break;
            }
        }
    }

    file.close();

    if(!isPrimary && !isLogic)
    {
        Consola::reportarError("No existe ninguna particion con el nombre: " + name);
        result = -1;
    }

    return result;
}

int DiskManager::mount(const std::string& path, const std::string& name)
{
    //Conseguir que particion hay que montar
    RaidOneFile file(path, std::ios::binary | std::ios::out | std::ios::in);
    if(!file.areOpen())
    {
        Consola::reportarError("Direccion: " + path + " no es valida");
        return -1;
    }
    Mbr mbr;
    file.read((char*)&mbr, sizeof(Mbr));

    char diskChar = 0;
    int partCount = -1;
    for (auto it: mountedPartitions) {
        if (it.second.path == path) {
            if(it.second.name == name){
                Consola::reportarError("La particion montada con id: " + it.first + " tiene el mismo nombre y  disco");
                return -1;
            }
            diskChar = it.first[2];//Chapuz: hardcoded 2
            partCount = std::max(partCount, std::stoi(it.first.substr(3)) + 1);
        }
    }
    static char diskCount = 'a';//bug si se montan tantas paraciones que se pasa de los printable chars
    if (diskChar == 0) {
        diskChar = diskCount++;
        partCount = 1;
    }

    //Revisar si es primaria:
    Partition* p = mbr.primaryPartition;
    int i;
    for(i = 0; i < PRIMARY_COUNT; i++)
    {
        if(p->name == name)
        {
            std::string mountedId = std::string("vd") + diskChar + std::to_string(partCount);
            mountedPartitions[mountedId] = MountedPart(*p, path, MountedPart::T_PRIMARY);
            ExtManager::updateMountTime(&file, mountedPartitions[mountedId]);//BLEH: busquedas no necesarias de mountedParts[name]
            file.close();
            return 0;
        }
        p++;
    }
    p = nullptr;

    //Revisar si es extendida
    if(mbr.extendedPartition.name == name){
        std::string mountedId = std::string("vd") + diskChar + std::to_string(partCount);
        mountedPartitions[mountedId] = MountedPart(mbr.extendedPartition, path, MountedPart::T_EXTENDED);
        ExtManager::updateMountTime(&file, mountedPartitions[mountedId]);//BLEH: busquedas no necesarias de mountedParts[name]
        file.close();
        return 0;
    }

    //Revisar si es logica
    //Busca entre las logica si no encontro una con el mismo nombre en las primarias y existen una particion extendida
    //Quitar comentario cuando se puedan hacer formateos en las logicas
    if(!mbr.extendedPartition.isNull())
    {
        //TODO: pasar a diskEntity
        Ebr currEbr;//Ebr que se va a eliminar
        file.seekg(mbr.extendedPartition.start, std::ios::beg);
        file.read((char*)&currEbr, sizeof(Ebr));//currEbr seria igual a topEbr y preEbr seria null por ahora
        while(!currEbr.isNull())//Se supone que la primera corrida de este while nunca va a dar verdadero porque va usar al topEbr como curr
        {
            if(strcmp(currEbr.name, name.c_str()) == 0)
            {
                std::string mountedId = std::string("vd") + diskChar + std::to_string(partCount);
                mountedPartitions[mountedId] = MountedPart(currEbr, path);
                ExtManager::updateMountTime(&file, mountedPartitions[mountedId]);//BLEH: busquedas no necesarias de mountedParts[name]
                file.close();
                return 0;
            }
            if(currEbr.next < 0){
                currEbr = Ebr();//Could be a break????
            }else{
                file.seekg(currEbr.next, std::ios::beg);
                file.read((char*)&currEbr, sizeof(Ebr));
            }
        }
    }

    Consola::reportarError("El disco: " + path + " no contiene una particion con el nombre: " + name);

    file.close();
    //Significa que estaba tratando de agrear a un nuevo y disco. Entonces incremento el diskCount sin necesidad.
    if(partCount == 1){
        diskCount--;
    }
    return -1;
}

int DiskManager::unMount(const std::string& id)
{
    auto iter = mountedPartitions.find(id);
    if (iter != mountedPartitions.end()) {
        ExtManager::updateUnmountTime(iter->second);
        mountedPartitions.erase(iter);
    }
    else {
        Consola::reportarError("No se ha montado una particion con el id: " + id);
        return -1;
    }
    return 0;
}

//TODO TODO TODO: Talvez fue este metodo el que causo los errores en la calificacion de la fase 1
std::optional<MountedPart> DiskManager::getMountedPartition(const std::string& id)
{
    auto iter = mountedPartitions.find(id);
    if (iter == mountedPartitions.end()) {
        return {};
    }

    return iter->second;
}
