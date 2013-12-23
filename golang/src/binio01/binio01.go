package main

import (
    "os"
    "fmt"
    "log"
    "encoding/binary"
    "strings"
    "strconv"
    "unsafe"
)

var g_verbose int = 0

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
    fmt.Print("\tBS_jmpBoot [")
    for i, b := range chdr.BS_jmpBoot {
        if 0 < i {
            fmt.Print(" ")
        }
        fmt.Printf("%#02x", b)
    }
    fmt.Println("]")

    fmt.Printf("\tBS_OEMName %q\n", cstr(chdr.BS_OEMName[:]))
    fmt.Printf("\tBPB_BytesPerSec %d\n", chdr.BPB_BytesPerSec)
    fmt.Printf("\tBPB_SecPerClus %d\n", chdr.BPB_SecPerClus)
    fmt.Printf("\tBPB_RsvdSecCnt %d\n", chdr.BPB_RsvdSecCnt)
    fmt.Printf("\tBPB_NumFATs %d\n", chdr.BPB_NumFATs)
    fmt.Printf("\tBPB_RootEntCnt %d\n", chdr.BPB_RootEntCnt)
    fmt.Printf("\tBPB_TotSec16 %d\n", chdr.BPB_TotSec16)
    fmt.Printf("\tBPB_Media %#02x\n", chdr.BPB_Media)
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

type DirEnt struct {
    DIR_Name [11]uint8
    DIR_Attr uint8
    DIR_NTRes uint8
    DIR_CrtTimeTenth uint8
    DIR_CrtTime uint16
    DIR_CrtDate uint16
    DIR_LstAccDate uint16
    DIR_FstClusHI uint16
    DIR_WrtTime uint16
    DIR_WrtDate uint16
    DIR_FstClusLO uint16
    DIR_FileSize uint32
}

func (de *DirEnt) Print() {
    fmt.Println("{")
    fmt.Printf("\tDIR_Name %q\n", cstr(de.DIR_Name[:]))
    if 0 < g_verbose {
        fmt.Printf("\tDIR_Attr 0b%s\n", strconv.FormatInt(int64(de.DIR_Attr), 2))
        if de.DIR_Attr == 0x0f {
            fmt.Println("\t\tLNF")
        } else {
            attrNames := []string{ "R/O", "Sys", "Hid", "Vol", "Dir", "Acv" }
            for i, nam := range attrNames {
                fmt.Printf("\t\t bit%d(%s) %d\n", i, nam, (de.DIR_Attr >> uint8(i)) & 0x01)
            }
        }
        fmt.Printf("\tDIR_NTRes %d\n", de.DIR_NTRes)
        fmt.Printf("\tDIR_CrtTimeTenth %d\n", de.DIR_CrtTimeTenth)
        fmt.Printf("\tDIR_CrtTime %d\n", de.DIR_CrtTime)
        fmt.Printf("\tDIR_CrtDate %d\n", de.DIR_CrtDate)
        fmt.Printf("\tDIR_LstAccDate %d\n", de.DIR_LstAccDate)
        fmt.Printf("\tDIR_FstClusHI %d\n", de.DIR_FstClusHI)
        fmt.Printf("\tDIR_WrtTime %d\n", de.DIR_WrtTime)
        fmt.Printf("\tDIR_WrtDate %d\n", de.DIR_WrtDate)
        fmt.Printf("\tDIR_FstClusLO %d\n", de.DIR_FstClusLO)
        fmt.Printf("\t\tcluster %d\n", de.Cluster())
        fmt.Printf("\tDIR_FileSize %d\n", de.DIR_FileSize)
    } else {
        if de.DIR_Attr == 0x0f {
            fmt.Println("\t\tLNF")
        }
    }
    fmt.Println("}")
}

func (de *DirEnt) Cluster() uint32 {
    return (uint32(de.DIR_FstClusHI) << 16) | uint32(de.DIR_FstClusLO)
}

func (de *DirEnt) IsDir() bool {
    return 0 != ((de.DIR_Attr >> 4) & 0x01)
}

func main() {
    fp, err := os.Open(os.Args[1])
    if err != nil {
        log.Fatal(err)
    }
    defer fp.Close()

    if 2 < len(os.Args) {
        g_verbose = 1
    }

    var chdr FATCommonHdr
    err = binary.Read(fp, binary.LittleEndian, &chdr)
    if err != nil {
        log.Fatal(err)
    }

    fmt.Println("{")
    chdr.Print()
    fmt.Println()

    var fat32 FAT32
    err = binary.Read(fp, binary.LittleEndian, &fat32)
    if err != nil {
        log.Fatal(err)
    }
    fat32.Print()
    fmt.Println()

    var fat12 FAT12
    err = binary.Read(fp, binary.LittleEndian, &fat12)
    if err != nil {
        log.Fatal(err)
    }
    fat12.Print()

    fmt.Println("}")
    fmt.Println()
//  fmt.Printf("%d + %d + %d == %d\n", unsafe.Sizeof(chdr), unsafe.Sizeof(fat32), unsafe.Sizeof(fat12),
//      unsafe.Sizeof(chdr) + unsafe.Sizeof(fat32) + unsafe.Sizeof(fat12))

    fatStartSector := uint32(chdr.BPB_RsvdSecCnt)
    fatSectors := uint32(chdr.BPB_FATSz16) * uint32(chdr.BPB_NumFATs)
    if fatSectors == 0 {
        fatSectors = fat32.BPB_FATSz32 * uint32(chdr.BPB_NumFATs)
    }
    fmt.Printf("fatStartSector %d\n", fatStartSector)
    fmt.Printf("fatSectors %d\n", fatSectors)

    rootDirStartSector := uint32(fatStartSector) + fatSectors
    const DirEntSz uint32 = 32
    rootDirSectors := (DirEntSz * uint32(chdr.BPB_RootEntCnt) + uint32(chdr.BPB_BytesPerSec) - 1) / uint32(chdr.BPB_BytesPerSec)
    fmt.Printf("rootDirStartSector %d\n", rootDirStartSector)
    fmt.Printf("rootDirSectors %d\n", rootDirSectors)

    dataStartSector := rootDirStartSector + rootDirSectors
    dataSectors := chdr.BPB_TotSec32 - dataStartSector
    fmt.Printf("dataStartSector %d\n", dataStartSector)
    fmt.Printf("dataSectors %d\n", dataSectors)

    fp.Seek(int64(fatStartSector) * int64(chdr.BPB_BytesPerSec), 0)
    fmt.Println("FAT")
    fatArr := make([]uint32, fatSectors * uint32(chdr.BPB_BytesPerSec) / 4)
    err = binary.Read(fp, binary.LittleEndian, &fatArr)
    if err != nil {
        log.Fatal(err)
    }

    const FATMask uint32 = (1 << 28) - 1
    for k, e := range fatArr {
        fmt.Printf("%x ", e & FATMask)
        if (k+1) % 32 == 0 {
            fmt.Println()
        }
    }
    fmt.Println()

    dirFatEntries := make([]uint32, 0)
//  rootDirFatEntry := fatArr[chdr.BPB_RootEntCnt] & FATMask
//  fmt.Printf("root FAT %#x\n", rootDirFatEntry)
    fp.Seek(int64(dataStartSector) * int64(chdr.BPB_BytesPerSec), 0)
    for {
        var de DirEnt
        err = binary.Read(fp, binary.LittleEndian, &de)
        if err != nil {
            log.Fatal(err)
        }
        de.Print()
        if de.IsDir() {
            dirFatEntries = append(dirFatEntries, de.Cluster())
        }
        if de.DIR_Name[0] == '\x00' {
            break
        }
    }
    fmt.Println()

    for _, clusterNo := range dirFatEntries {
        fmt.Printf("dir contents is at %d\n", clusterNo)
        fp.Seek(int64(dataStartSector + (clusterNo-2) * uint32(chdr.BPB_SecPerClus)) * int64(chdr.BPB_BytesPerSec), 0)
        for {
            var de DirEnt
            for i := uint16(0); i < uint16(chdr.BPB_SecPerClus) * chdr.BPB_BytesPerSec / uint16(unsafe.Sizeof(de)); i++ {
                err = binary.Read(fp, binary.LittleEndian, &de)
                if err != nil {
                    log.Fatal(err)
                }
                de.Print()
                if de.DIR_Name[0] == '\x00' {
                    break
                }
            }
            clusterNo = fatArr[clusterNo] & FATMask
            if 0xffffff8 <= clusterNo && clusterNo <= 0xfffffff {
                break
            }
            fmt.Printf("continues to %d\n", clusterNo)
            fp.Seek(int64(dataStartSector + (clusterNo-2) * uint32(chdr.BPB_SecPerClus)) * int64(chdr.BPB_BytesPerSec), 0)
        }
    }
}
