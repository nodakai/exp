package main

import (
    "os"
    "fmt"
    "log"
    "encoding/binary"
    "strings"
)

type FATCommonHdr struct {
    BS_jmpBoot [3]uint8
    BS_OEMName [8]uint8
    BPB_BytesPerSec uint16
    BPB_SecPerClus uint8
    BPB_RsvdSecCnt uint16
    BPB_NumFATs uint8
    BPB_RootEntCnt uint16
    BPB_TotSec16 uint16
    BPB_Media uint8
    BPB_FATSz16 uint16
    BPB_SecPerTrk uint16
    BPB_NumHeads uint16
    BPB_HiddSec uint32
    BPB_TotSec32 uint32
}

func cstr(s []uint8) string {
    return strings.TrimRight(string(s), "\x00")
}

func (chdr *FATCommonHdr) Print() {
    print("\tBS_jmpBoot [")
    for i, b := range chdr.BS_jmpBoot {
        if 0 < i {
            print(" ")
        }
        fmt.Printf("%#02x", b)
    }
    println("]")

    fmt.Printf("\tBS_OEMName %q\n", cstr(chdr.BS_OEMName[:]))
    fmt.Printf("\tBPB_BytesPerSec %d\n", chdr.BPB_BytesPerSec)
    fmt.Printf("\tBPB_SecPerClus %d\n", chdr.BPB_SecPerClus)
    fmt.Printf("\tBPB_RsvdSecCnt %d\n", chdr.BPB_RsvdSecCnt)
    fmt.Printf("\tBPB_NumFATs %d\n", chdr.BPB_NumFATs)
    fmt.Printf("\tBPB_RootEntCnt %d\n", chdr.BPB_RootEntCnt)
    fmt.Printf("\tBPB_TotSec16 %d\n", chdr.BPB_TotSec16)
    fmt.Printf("\tBPB_Media %d\n", chdr.BPB_Media)
    fmt.Printf("\tBPB_FATSz16 %d\n", chdr.BPB_FATSz16)
    fmt.Printf("\tBPB_SecPerTrk %d\n", chdr.BPB_SecPerTrk)
    fmt.Printf("\tBPB_NumHeads %d\n", chdr.BPB_NumHeads)
    fmt.Printf("\tBPB_HiddSec %d\n", chdr.BPB_HiddSec)
    fmt.Printf("\tBPB_TotSec32 %d\n", chdr.BPB_TotSec32)
}

type FAT32 struct {
    BPB_FATSz32 uint32
    BPB_ExtFlags uint16
    BPB_FSVer uint16
    BPB_RootClus uint32
    BPB_FSInfo uint16
    BPB_BkBootSec uint16
    BPB_Reserved [12]uint8
}

func (fat *FAT32) Print() {
    fmt.Printf("\tBPB_FATSz32 %d\n", fat.BPB_FATSz32)
    fmt.Printf("\tBPB_ExtFlags %d\n", fat.BPB_ExtFlags)
    fmt.Printf("\tBPB_FSVer %d\n", fat.BPB_FSVer)
    fmt.Printf("\tBPB_RootClus %d\n", fat.BPB_RootClus)
    fmt.Printf("\tBPB_FSInfo %d\n", fat.BPB_FSInfo)
    fmt.Printf("\tBPB_BkBootSec %d\n", fat.BPB_BkBootSec)
    fmt.Printf("\tBPB_Reserved %q\n", cstr(fat.BPB_Reserved[:]))
}

type FAT12 struct {
    BS_DrvNum uint8
    BS_Reserved1 uint8
    BS_BootSig uint8
    BS_VolID uint32
    BS_VolLab [11]uint8
    BS_FilSysType [8]uint8
}

func (fat *FAT12) Print() {
    fmt.Printf("\tBS_DrvNum %d\n", fat.BS_DrvNum)
    fmt.Printf("\tBS_Reserved1 %d\n", fat.BS_Reserved1)
    fmt.Printf("\tBS_BootSig %d\n", fat.BS_BootSig)
    fmt.Printf("\tBS_VolID %d\n", fat.BS_VolID)
    fmt.Printf("\tBS_VolLab %q\n", cstr(fat.BS_VolLab[:]))
    fmt.Printf("\tBS_FilSysType %q\n", cstr(fat.BS_FilSysType[:]))
}

func main() {
    fp, err := os.Open("/tmp/fat32.img")
    if err != nil {
        log.Fatal(err)
    }
    defer fp.Close()

    var chdr FATCommonHdr
    err = binary.Read(fp, binary.LittleEndian, &chdr)
    if err != nil {
        log.Fatal(err)
    }

    println("{")
    chdr.Print()
    println()

    var fat32 FAT32
    err = binary.Read(fp, binary.LittleEndian, &fat32)
    if err != nil {
        log.Fatal(err)
    }
    fat32.Print()
    println()

    var fat12 FAT12
    err = binary.Read(fp, binary.LittleEndian, &fat12)
    if err != nil {
        log.Fatal(err)
    }
    fat12.Print()

    println("}")
    println()

    fatStartSector := uint32(chdr.BPB_RsvdSecCnt)
    fatSectors := uint32(chdr.BPB_FATSz16)
    if fatSectors == 0 {
        fatSectors = fat32.BPB_FATSz32
    }
    fmt.Printf("fatStartSector %d\n", fatStartSector)
    fmt.Printf("fatSectors %d\n", fatSectors)

    rootDirStartSector := uint32(fatStartSector) * fatSectors
    const DirEntSz uint32 = 32
    rootDirSectors := (DirEntSz * uint32(chdr.BPB_RootEntCnt) + uint32(chdr.BPB_BytesPerSec) - 1) / uint32(chdr.BPB_BytesPerSec)
    fmt.Printf("rootDirStartSector %d\n", rootDirStartSector)
    fmt.Printf("rootDirSectors %d\n", rootDirSectors)

    dataStartSector := fatStartSector + rootDirStartSector
    dataSectors := chdr.BPB_TotSec32 - dataStartSector
    fmt.Printf("dataStartSector %d\n", dataStartSector)
    fmt.Printf("dataSectors %d\n", dataSectors)

}
