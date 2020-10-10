/*
 * @Author: zhongwei
 * @Date: 2019/11/29 8:54:18
 * @Description: 从Altera EDS 16.1复制过来的MMU驱动代码
 * @File: hd_mmu.h
 *
*/

/******************************************************************************
*
* Copyright 2013 Altera Corporation. All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

/*
 * $Id: //acds/rel/16.1/embedded/ip/hps/altera_hps/hwlib/include/alt_mmu.h#1 $
 */

/*! \file
 *  Altera - MMU Management API
 */

#ifndef __ALT_MMU_H__
#define __ALT_MMU_H__

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

/******************************************************************************/
/*! \addtogroup ALT_MMU MMU Management API
 *
 * This module defines an API for configuring and managing the Cortex-A9 MMU.
 *
 * The APIs in this module are divided into two categories:
 * * Support for low-level MMU configuration and operation.
 * * Support for simplified virtual address space definition and enablement.
 *
 * The functions in the low-level MMU API provide capabilities to:
 * * Control and maintain the MMU operational state.
 * * Create and maintain MMU translation tables using a low-level API.
 *
 * The low-level API does not directly support any particular virtual address
 * implementation model. Many features of the MMU hardware are oriented toward
 * efficient implementation of protected virtual addressing in a multi-tasking
 * operating system environment.
 *
 * While the functions in the low-level MMU API could be used to facilitate a port
 * to an operating system exploiting these MMU features, the API itself does not
 * directly implement any particular virtual address implementation model or
 * policy.
 *
 * The other API does directly support a simplified virtual address space
 * implementation model. This API provides a client facility to programmatically
 * define a simplified virtual address space from a set of high level memory
 * region configurations. The API also provides a convenient method to enable the
 * virtual address space once it is defined.
 *
 * For a complete understanding of the possible configurations and operation of
 * the MMU, consult the following references:
 * * <em>ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition (ARM DDI
 *   0406C), Chapter B3 Virtual Memory System Architecture (VMSA)</em>
 * * <em>ARM Cortex-A9 Technical Reference Manual(ARM DDI 0388G), Chapter 6 Memory
 *   Management Unit</em>
 *
 * @{
 */

/*!
 * This type enumerates the options for Shareability (S) properties in translation
 * table descriptors. This control determines whether the addressed region is
 * Shareable memory or not.
 *
 * The Shareability property (S bit):
 * * Is ignored if the entry refers to Device or Strongly-ordered memory.
 * * For Normal memory, determines whether the memory region is Shareable or Non-shareable:
 *   - S == 0 Normal memory region is Non-shareable.
 *   - S == 1 Normal memory region is Shareable.
 */
typedef enum ALT_MMU_TTB_S_e
{
    ALT_MMU_TTB_S_NON_SHAREABLE = 0,    /*!< Non-Shareable address map */
    ALT_MMU_TTB_S_SHAREABLE     = 1     /*!< Shareable address map     */
} ALT_MMU_TTB_S_t;


/*!
 * This type enumerates the options for Non-Secure (NS) controls in translation
 * table descriptors. This control specifies whether memory accesses made from the
 * secure state translate physical address in the secure or non-secure address
 * map. The value of the NS bit in the first level page table descriptor applies
 * to all entries in the corresponding second-level translation table.
 */
typedef enum ALT_MMU_TTB_NS_e
{
    ALT_MMU_TTB_NS_SECURE     = 0,      /*!< Secure address map     */
    ALT_MMU_TTB_NS_NON_SECURE = 1       /*!< Non-Secure address map */
} ALT_MMU_TTB_NS_t;

/*!
 * This type enumerates the options for Execute Never (XN) controls in translation
 * table descriptors that determine whether the processor can execute instructions
 * from the addressed region.
 */
typedef enum ALT_MMU_TTB_XN_e
{
    ALT_MMU_TTB_XN_DISABLE    = 0,      /*!< Instructions can be executed from
                                         *   this memory region.
                                         */
    ALT_MMU_TTB_XN_ENABLE     = 1       /*!< Instructions cannot be executed from
                                         *   this memory region. A permission
                                         *   fault is generated if an attempt to
                                         *   execute an instruction from this
                                         *   memory region. However, if using the
                                         *   short-descriptor translation table
                                         *   format, the fault is generated only
                                         *   if the access is to memory in the
                                         *   client domain.
                                         */
} ALT_MMU_TTB_XN_t;

/*!
 * This type enumerates the Domain Access Permission (DAP) options that can be set
 * in the Domain Access Control Register (DACR).
 */
typedef enum ALT_MMU_DAP_e
{
    ALT_MMU_DAP_NO_ACCESS     = 0x0,    /*!< No access. Any access to the domain
                                         *   generates a Domain fault.
                                         */
    ALT_MMU_DAP_CLIENT        = 0x1,    /*!< Client. Accesses are checked against
                                         *   the permission bits in the
                                         *   translation tables.
                                         */
    ALT_MMU_DAP_RESERVED      = 0x2,    /*!< Reserved, effect is UNPREDICTABLE. */
    ALT_MMU_DAP_MANAGER       = 0x3     /*!< Manager. Accesses are not checked
                                         *   against the permission bits in the
                                         *   translation tables.
                                         */
} ALT_MMU_DAP_t;

/*!
 * This type enumerates the Access Permissions that can be specified for a memory
 * region.
 *
 * Memory access control is defined using access permission bits in translation
 * table descriptors that control access to the corresponding memory region.
 *
 * The HWLIB uses the short-descriptor translation table format for defining the
 * access permissions where three bits, AP[2:0], define the access
 * permissions. The SCTLR.AFE must be set to 0.
 *
 * The following table provides a summary of the enumerations, AP bit encodings,
 * and access permission descriptions for this type.
 *
 *  Enumeration               | AP Value | Privileged (PL1) Access | User (PL0) Access | Description
 * :--------------------------|:---------|:------------------------|:------------------|:-------------------------------------
 *  ALT_MMU_AP_NO_ACCESS      | 000      | No Access               | No Access         | No Access
 *  ALT_MMU_AP_PRIV_ACCESS    | 001      | Read/Write              | No Access         | Privileged access only
 *  ALT_MMU_AP_USER_READ_ONLY | 010      | Read/Write              | Read Only         | Write in user mode generates a fault
 *  ALT_MMU_AP_FULL_ACCESS    | 011      | Read/Write              | Read/Write        | Full Access
 *  N/A                       | 100      | Unknown                 | Unknown           | Reserved
 *  ALT_MMU_AP_PRIV_READ_ONLY | 101      | Read Only               | No Access         | Privileged read only
 *  N/A                       | 110      | Read Only               | Read Only         | Read Only - deprecated
 *  ALT_MMU_AP_READ_ONLY      | 111      | Read Only               | Read Only         | Read Only
 */
enum ALT_MMU_AP_e
{
    ALT_MMU_AP_NO_ACCESS      = 0, /*!< No Access                            */
    ALT_MMU_AP_PRIV_ACCESS    = 1, /*!< Privileged access only               */
    ALT_MMU_AP_USER_READ_ONLY = 2, /*!< Write in user mode generates a fault */
    ALT_MMU_AP_FULL_ACCESS    = 3, /*!< Full Access                          */
    ALT_MMU_AP_PRIV_READ_ONLY = 5, /*!< Privileged read only                 */
    ALT_MMU_AP_READ_ONLY      = 7  /*!< Read Only                            */
};
/*! Typedef name for enum ALT_MMU_AP_e */
typedef enum ALT_MMU_AP_e       ALT_MMU_AP_t;

/*!
 * This type enumerates the Memory Region attributes that can be specifed in MMU
 * translation table entries. Memory attributes determine the memory ordering and
 * cache policies for inner/outer domains used for a particular range of memory.
 *
 * Within the translation table entries, the memory region attributes are encoded
 * using a combination of the descriptor entry data fields (TEX, C, B).  Memory
 * attribute settings also affect the meaning of other memory region properties
 * such as shareability (S).
 *
 * The tables below describe the available enumerations for specifying different
 * memory region attributes and their affect on shareability.
 *
 * The memory attributes enumerated here are meant to be used is a system where
 * TEX remap is disabled (i.e. SCTLR.TRE is set to 0).
 *
 *  Enumeration            | TEX | C | B | Description                                       | Shareability
 * :-----------------------|:----|:--|:--|:--------------------------------------------------|:---------------------------------
 *  ALT_MMU_ATTR_STRONG    | 000 | 0 | 0 | Strongly Ordered                                  | Shareable
 *  ALT_MMU_ATTR_DEVICE    | 000 | 0 | 1 | Device                                            | Shareable
 *  ALT_MMU_ATTR_WT        | 000 | 1 | 0 | Inner/Outer Write-Through, No Write Allocate      | Determined by descriptor [S] bit
 *  ALT_MMU_ATTR_WB        | 000 | 1 | 1 | Inner/Outer Write-Back, No Write Allocate         | Determined by descriptor [S] bit
 *  ALT_MMU_ATTR_NC        | 001 | 0 | 0 | Inner/Outer Non-Cacheable                         | Determined by descriptor [S] bit
 *  N/A                    | 001 | 0 | 1 | Reserved                                          | Reserved
 *  N/A                    | 001 | 1 | 0 | Implementation Defined                            | -
 *  ALT_MMU_ATTR_WBA       | 001 | 1 | 1 | Inner/Outer Write-Back, Write Allocate            | Determined by descriptor [S] bit
 *  ALT_MMU_ATTR_DEVICE_NS | 010 | 0 | 0 | Device                                            | Non-Shareable
 *  N/A                    | 010 | 0 | 1 | Reserved                                          | Reserved
 *  N/A                    | 010 | 1 | 0 | Reserved                                          | Reserved
 *  N/A                    | 010 | 1 | 1 | Reserved                                          | Reserved
 *  ALT_MMU_ATTR_AA_BB     | 1BB | A | A | Cached where AA = Inner Policy, BB = Outer Policy | Determined by descriptor [S] bit
 *
 *  Cache Policy Encoding for AA, BB
 *
 *  Mnemonic Encoding | Bit Encoding | Cache Policy
 * :------------------|:-------------|:---------------------------------
 *  NC                | 00           | Non-Cacheable
 *  WBA               | 01           | Write-Back, Write Allocate
 *  WT                | 10           | Write-Through, No Write Allocate
 *  WB                | 11           | Write-Back, No Write Allocate
 *
 * \internal
 * The encoding of the enum values is that the MSB 4 bits is TEX while the
 * LSB is C | B. Fault is 0xff. This makes it easier to decode TEX, C, B from
 * the actual enum value.
 * \endinternal
 */
enum ALT_MMU_ATTR_e
{
    ALT_MMU_ATTR_FAULT     = 0xff, /*!< Generates fault descriptor entries for memory region */
    ALT_MMU_ATTR_STRONG    = 0x00, /*!< Strongly Ordered Shareable */
    ALT_MMU_ATTR_DEVICE    = 0x01, /*!< Device Shareable */
    ALT_MMU_ATTR_WT        = 0x02, /*!< Inner/Outer Write-Through, No Write Allocate, Shareability determined by [S] */
    ALT_MMU_ATTR_WB        = 0x03, /*!< Inner/Outer Write-Back, No Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_NC        = 0x10, /*!< Inner/Outer Non-Cacheable, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WBA       = 0x13, /*!< Inner/Outer Write-Back, Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_DEVICE_NS = 0x20, /*!< Device Non-Shareable */

    ALT_MMU_ATTR_NC_NC     = 0x40, /*!< Inner Non-Cacheable, Outer Non-Cacheable, Shareability determined by [S] bit */
    ALT_MMU_ATTR_NC_WBA    = 0x50, /*!< Inner Non-Cacheable, Outer Write-Back Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_NC_WT     = 0x60, /*!< Inner Non-Cacheable, Outer Write-Through, Shareability determined by [S] bit */
    ALT_MMU_ATTR_NC_WB     = 0x70, /*!< Inner Non-Cacheable, Outer Write-Back, Shareability determined by [S] bit */

    ALT_MMU_ATTR_WBA_NC    = 0x41, /*!< Inner Write-Back Write Allocate, Outer Non-Cacheable, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WBA_WBA   = 0x51, /*!< Inner Write-Back Write Allocate, Outer Write-Back Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WBA_WT    = 0x61, /*!< Inner Write-Back Write Allocate, Outer Write-Through, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WBA_WB    = 0x71, /*!< Inner Write-Back Write Allocate, Outer Write-Back, Shareability determined by [S] bit */

    ALT_MMU_ATTR_WT_NC     = 0x42, /*!< Inner Write-Through, Outer Non-Cacheable, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WT_WBA    = 0x52, /*!< Inner Write-Through, Outer Write-Back Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WT_WT     = 0x62, /*!< Inner Write-Through, Outer Write-Through, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WT_WB     = 0x72, /*!< Inner Write-Through, Outer Write-Back, Shareability determined by [S] bit */

    ALT_MMU_ATTR_WB_NC     = 0x43, /*!< Inner Write-Back, Outer Non-Cacheable, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WB_WBA    = 0x53, /*!< Inner Write-Back, Outer Write-Back Write Allocate, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WB_WT     = 0x63, /*!< Inner Write-Back, Outer Write-Through, Shareability determined by [S] bit */
    ALT_MMU_ATTR_WB_WB     = 0x73 /*!< Inner Write-Back, Outer Write-Back, Shareability determined by [S] bit */

};
/*! Typedef name for enum ALT_MMU_ATTR_e */
typedef enum ALT_MMU_ATTR_e     ALT_MMU_ATTR_t;


/******************************************************************************/
/*! \addtogroup ALT_MMU_MGMT MMU Management
 *
 * This section defines low-level declarations, macros, and functions for creating
 * and maintaining MMU first and second level translation tables and their short
 * descriptor entries.
 *
 * The basic functions to enable/disable and configure the opertational state of
 * the MMU are in this section.
 *
 * The operations in this section are for users that want to exercise a fine
 * degree of configuration and control over the MMU. It requires a more detailed
 * understanding of the MMU, its different modes of operation, and puts more
 * responsibility on the user for correct functioning.
 *
 * Users desiring basic configuration and enablement of the MMU to support a
 * virtual address space should use the operations in the section \ref ALT_MMU_VA
 * "MMU Virtual Address Space Creation".
 *
 * @{
 */

/*!
 * The size of a supersection in bytes is 16 MiB.
 */
#define ALT_MMU_SUPERSECTION_SIZE               (1UL << 24)

/*!
 * The size of a section in bytes is 1 MiB.
 */
#define ALT_MMU_SECTION_SIZE                    (1UL << 20)

/*!
 * The size of a large page in bytes is 64 KiB.
 */
#define ALT_MMU_LARGE_PAGE_SIZE                 (1UL << 16)

/*!
 * The size of a small page in bytes is 4 KiB.
 */
#define ALT_MMU_SMALL_PAGE_SIZE                 (1UL << 12)

/*!
 * The size of a first level translation table for the short descriptor format in
 * bytes.
 */
#define ALT_MMU_TTB1_SIZE                       16384

/*!
 * The size of a second level translation table for the short descriptor format in
 * bytes.
 */
#define ALT_MMU_TTB2_SIZE                       1024

/******************************************************************************/
/*! \addtogroup ALT_MMU_MGMT_MACRO_TTB1 MMU Management Macros - First Level Translation Table
 *
 * The macro definitions in this section support access to the short-descriptor
 * first-level table entries and their constituent fields.
 *
 * These macros may be used to create descriptor entry values that are passed to a
 * first level translation table contruction function such as
 * alt_mmu_ttb1_desc_set().
 *
 * Each short-descriptor has a set of macro definitions of the following form:
 *
 * * \b ALT_MMU_TTB1_<type_and_field_name>_MASK - bit mask for the descriptor type
 *                                                and field.
 *
 * * \b ALT_MMU_TTB1_<type_and_field_name>_GET(desc) - extracts the field value
 *                                                     from the descriptor entry \e
 *                                                     desc.
 *
 * * \b ALT_MMU_TTB1_<type_and_field_name>_SET(val) - returns a field \e val
 *                                                    shifted and masked that is
 *                                                    suitable for setting a
 *                                                    descriptor entry.
 *
 * @{
 */

/*!
 *
 */
#define ALT_MMU_TTB1_TYPE_MASK                            0x00000003
#define ALT_MMU_TTB1_TYPE_GET(desc)                       (((desc) & ALT_MMU_TTB1_TYPE_MASK) >> 0)
#define ALT_MMU_TTB1_TYPE_SET(val)                        (((val) << 0) & ALT_MMU_TTB1_TYPE_MASK)

/*!
 * \name First Level Translation Table Page Table Entry [NS]
 *
 * The Non-Secure [NS] bit. This bit specifies whether the translated PA is in
 * the Secure or Non-Secure address map.
 * @{
 */
#define ALT_MMU_TTB1_PAGE_TBL_NS_MASK                     0x00000008
#define ALT_MMU_TTB1_PAGE_TBL_NS_GET(desc)                (((desc) & ALT_MMU_TTB1_PAGE_TBL_NS_MASK) >> 3)
#define ALT_MMU_TTB1_PAGE_TBL_NS_SET(val)                 (((val) << 3) & ALT_MMU_TTB1_PAGE_TBL_NS_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Page Table Entry [DOMAIN]
 *
 * Domain field. Page table descriptor applies to all entries in the corresponding
 * second-level translation table.
 * @{
 */
#define ALT_MMU_TTB1_PAGE_TBL_DOMAIN_MASK                 0x000001e0
#define ALT_MMU_TTB1_PAGE_TBL_DOMAIN_GET(desc)            (((desc) & ALT_MMU_TTB1_PAGE_TBL_DOMAIN_MASK) >> 5)
#define ALT_MMU_TTB1_PAGE_TBL_DOMAIN_SET(val)             (((val) << 5) & ALT_MMU_TTB1_PAGE_TBL_DOMAIN_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Page Table Entry Page Table Base Address
 * @{
 */
#define ALT_MMU_TTB1_PAGE_TBL_BASE_ADDR_MASK              0xfffffc00
#define ALT_MMU_TTB1_PAGE_TBL_BASE_ADDR_GET(desc)         (((desc) & ALT_MMU_TTB1_PAGE_TBL_BASE_ADDR_MASK) >> 10)
#define ALT_MMU_TTB1_PAGE_TBL_BASE_ADDR_SET(val)          (((val) << 10) & ALT_MMU_TTB1_PAGE_TBL_BASE_ADDR_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [B]
 *
 * The [B] field of the memory region attributes. [B] is an arcane reference to
 * Bufferable attribute.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_B_MASK                       0x00000004
#define ALT_MMU_TTB1_SECTION_B_GET(desc)                  (((desc) & ALT_MMU_TTB1_SECTION_B_MASK) >> 2)
#define ALT_MMU_TTB1_SECTION_B_SET(val)                   (((val) << 2) & ALT_MMU_TTB1_SECTION_B_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [C]
 *
 * The [C] field of the memory region attributes. [C] is an arcane reference to
 * Cacheable attribute.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_C_MASK                       0x00000008
#define ALT_MMU_TTB1_SECTION_C_GET(desc)                  (((desc) & ALT_MMU_TTB1_SECTION_C_MASK) >> 3)
#define ALT_MMU_TTB1_SECTION_C_SET(val)                   (((val) << 3) & ALT_MMU_TTB1_SECTION_C_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [XN]
 *
 * The Execute-Never bit. Determines whether the processor can execute software
 * from the addressed region.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_XN_MASK                      0x00000010
#define ALT_MMU_TTB1_SECTION_XN_GET(desc)                 (((desc) & ALT_MMU_TTB1_SECTION_XN_MASK) >> 4)
#define ALT_MMU_TTB1_SECTION_XN_SET(val)                  (((val) << 4) & ALT_MMU_TTB1_SECTION_XN_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [DOMAIN]
 *
 * Domain field.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_DOMAIN_MASK                  0x000001e0
#define ALT_MMU_TTB1_SECTION_DOMAIN_GET(desc)             (((desc) & ALT_MMU_TTB1_SECTION_DOMAIN_MASK) >> 5)
#define ALT_MMU_TTB1_SECTION_DOMAIN_SET(val)              (((val) << 5) & ALT_MMU_TTB1_SECTION_DOMAIN_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [AP]
 *
 * Access Permissions bits.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_AP_MASK                      0x00008c00
#define ALT_MMU_TTB1_SECTION_AP_GET(desc)                 ((((desc) & 0x00008000) >> 13) | (((desc) & 0x00000c00) >> 10))
#define ALT_MMU_TTB1_SECTION_AP_SET(val)                  ((((val) << 13) & 0x00008000) | (((val) << 10) & 0x00000c00))
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [TEX]
 *
 * The [TEX] field of the memory region attributes. [TEX] is an arcane reference to
 * Type EXtension attribute.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_TEX_MASK                     0x00007000
#define ALT_MMU_TTB1_SECTION_TEX_GET(desc)                (((desc) & ALT_MMU_TTB1_SECTION_TEX_MASK) >> 12)
#define ALT_MMU_TTB1_SECTION_TEX_SET(val)                 (((val) << 12) & ALT_MMU_TTB1_SECTION_TEX_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [S]
 *
 * The Shareable bit. Determines whether the addressed region is shareable memory.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_S_MASK                       0x00010000
#define ALT_MMU_TTB1_SECTION_S_GET(desc)                  (((desc) & ALT_MMU_TTB1_SECTION_S_MASK) >> 16)
#define ALT_MMU_TTB1_SECTION_S_SET(val)                   (((val) << 16) & ALT_MMU_TTB1_SECTION_S_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [nG]
 *
 * The not global bit. Determines how the translation is marked in the TLB.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_NG_MASK                      0x00020000
#define ALT_MMU_TTB1_SECTION_NG_GET(desc)                 (((desc) & ALT_MMU_TTB1_SECTION_NG_MASK) >> 17)
#define ALT_MMU_TTB1_SECTION_NG_SET(val)                  (((val) << 17) & ALT_MMU_TTB1_SECTION_NG_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry [NS]
 *
 * The Non-Secure [NS] bit. This bit specifies whether the translated PA is in
 * the Secure or Non-Secure address map.
 * @{
 */
#define ALT_MMU_TTB1_SECTION_NS_MASK                      0x00080000
#define ALT_MMU_TTB1_SECTION_NS_GET(desc)                 (((desc) & ALT_MMU_TTB1_SECTION_NS_MASK) >> 19)
#define ALT_MMU_TTB1_SECTION_NS_SET(val)                  (((val) << 19) & ALT_MMU_TTB1_SECTION_NS_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Section Entry Section Base Address
 * @{
 */
#define ALT_MMU_TTB1_SECTION_BASE_ADDR_MASK               0xfff00000
#define ALT_MMU_TTB1_SECTION_BASE_ADDR_GET(desc)          (((desc) & ALT_MMU_TTB1_SECTION_BASE_ADDR_MASK) >> 20)
#define ALT_MMU_TTB1_SECTION_BASE_ADDR_SET(val)           (((val) << 20) & ALT_MMU_TTB1_SECTION_BASE_ADDR_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [B]
 *
 * The [B] field of the memory region attributes. [B] is an arcane reference to
 * Bufferable attribute.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_B_MASK                  0x00000004
#define ALT_MMU_TTB1_SUPERSECTION_B_GET(desc)             (((desc) & ALT_MMU_TTB1_SUPERSECTION_B_MASK) >> 2)
#define ALT_MMU_TTB1_SUPERSECTION_B_SET(val)              (((val) << 2) & ALT_MMU_TTB1_SUPERSECTION_B_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [C]
 *
 * The [C] field of the memory region attributes. [C] is an arcane reference to
 * Cacheable attribute.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_C_MASK                  0x00000008
#define ALT_MMU_TTB1_SUPERSECTION_C_GET(desc)             (((desc) & ALT_MMU_TTB1_SUPERSECTION_C_MASK) >> 3)
#define ALT_MMU_TTB1_SUPERSECTION_C_SET(val)              (((val) << 3) & ALT_MMU_TTB1_SUPERSECTION_C_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [XN]
 *
 * The Execute-Never bit. Determines whether the processor can execute software
 * from the addressed region.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_XN_MASK                 0x00000010
#define ALT_MMU_TTB1_SUPERSECTION_XN_GET(desc)            (((desc) & ALT_MMU_TTB1_SUPERSECTION_XN_MASK) >> 4)
#define ALT_MMU_TTB1_SUPERSECTION_XN_SET(val)             (((val) << 4) & ALT_MMU_TTB1_SUPERSECTION_XN_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [DOMAIN]
 *
 * Domain field.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_DOMAIN_MASK             0x000001e0
#define ALT_MMU_TTB1_SUPERSECTION_DOMAIN_GET(desc)        (((desc) & ALT_MMU_TTB1_SUPERSECTION_DOMAIN_MASK) >> 5)
#define ALT_MMU_TTB1_SUPERSECTION_DOMAIN_SET(val)         (((val) << 5) & ALT_MMU_TTB1_SUPERSECTION_DOMAIN_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [AP]
 *
 * Access Permissions bits.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_AP_MASK                 0x00008c00
#define ALT_MMU_TTB1_SUPERSECTION_AP_GET(desc)            ((((desc) & 0x00008000) >> 13) | (((desc) & 0x00000c00) >> 10))
#define ALT_MMU_TTB1_SUPERSECTION_AP_SET(val)             ((((val) << 13) & 0x00008000) | (((val) << 10) & 0x00000c00))
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [TEX]
 *
 * The [TEX] field of the memory region attributes. [TEX] is an arcane reference to
 * Type EXtension attribute.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_TEX_MASK                0x00007000
#define ALT_MMU_TTB1_SUPERSECTION_TEX_GET(desc)           (((desc) & ALT_MMU_TTB1_SUPERSECTION_TEX_MASK) >> 12)
#define ALT_MMU_TTB1_SUPERSECTION_TEX_SET(val)            (((val) << 12) & ALT_MMU_TTB1_SUPERSECTION_TEX_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [S]
 *
 * The Shareable bit. Determines whether the addressed region is shareable memory.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_S_MASK                  0x00010000
#define ALT_MMU_TTB1_SUPERSECTION_S_GET(desc)             (((desc) & ALT_MMU_TTB1_SUPERSECTION_S_MASK) >> 16)
#define ALT_MMU_TTB1_SUPERSECTION_S_SET(val)              (((val) << 16) & ALT_MMU_TTB1_SUPERSECTION_S_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [nG]
 *
 * The not global bit. Determines how the translation is marked in the TLB.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_NG_MASK                 0x00020000
#define ALT_MMU_TTB1_SUPERSECTION_NG_GET(desc)            (((desc) & ALT_MMU_TTB1_SUPERSECTION_NG_MASK) >> 17)
#define ALT_MMU_TTB1_SUPERSECTION_NG_SET(val)             (((val) << 17) & ALT_MMU_TTB1_SUPERSECTION_NG_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry [NS]
 *
 * The Non-Secure [NS] bit. This bit specifies whether the translated PA is in
 * the Secure or Non-Secure address map.
 * @{
 */
#define ALT_MMU_TTB1_SUPERSECTION_NS_MASK                 0x00080000
#define ALT_MMU_TTB1_SUPERSECTION_NS_GET(desc)            (((desc) & ALT_MMU_TTB1_SUPERSECTION_NS_MASK) >> 19)
#define ALT_MMU_TTB1_SUPERSECTION_NS_SET(val)             (((val) << 19) & ALT_MMU_TTB1_SUPERSECTION_NS_MASK)
/*! @} */

/*!
 * \name First Level Translation Table Supersection Entry Supersection Base Address
 */
#define ALT_MMU_TTB1_SUPERSECTION_BASE_ADDR_MASK          0xff000000
#define ALT_MMU_TTB1_SUPERSECTION_BASE_ADDR_GET(desc)     (((desc) & ALT_MMU_TTB1_SUPERSECTION_BASE_ADDR_MASK) >> 24)
#define ALT_MMU_TTB1_SUPERSECTION_BASE_ADDR_SET(val)      (((val) << 24) & ALT_MMU_TTB1_SUPERSECTION_BASE_ADDR_MASK)
/*! @} */

/*! @} */

/******************************************************************************/
/*! \addtogroup ALT_MMU_MGMT_MACRO_TTB2 MMU Management Macros - Second Level Translation Table
 *
 * The macro definitions in this section support access to the short-descriptor
 * second-level table entries and their constituent fields.
 *
 * These macros may be used to create descriptor entry values that are passed to a
 * second level translation table contruction function such as
 * alt_mmu_ttb2_desc_set().
 *
 * Each short-descriptor has a set of macro definitions of the following form:
 *
 * * \b ALT_MMU_TTB2_<type_and_field_name>_MASK - bit mask for the descriptor type
 *                                                and field.
 *
 * * \b ALT_MMU_TTB2_<type_and_field_name>_GET(desc) - extracts the field value
 *                                                     from the descriptor entry \e
 *                                                     desc.
 *
 * * \b ALT_MMU_TTB2_<type_and_field_name>_SET(val) - returns a field \e val
 *                                                    shifted and masked that is
 *                                                    suitable for setting a
 *                                                    descriptor entry.
 * @{
 */
/*!
 *
 */
#define ALT_MMU_TTB2_TYPE_MASK                            0x00000003
#define ALT_MMU_TTB2_TYPE_GET(desc)                       (((desc) & ALT_MMU_TTB2_TYPE_MASK) >> 0)
#define ALT_MMU_TTB2_TYPE_SET(val)                        (((val) << 0) & ALT_MMU_TTB2_TYPE_MASK)

/*!
 * \name Second Level Translation Table Large Page Table Entry [B]
 *
 * The [B] field of the memory region attributes. [B] is an arcane reference to
 * Bufferable attribute.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_B_MASK                    0x00000004
#define ALT_MMU_TTB2_LARGE_PAGE_B_GET(desc)               (((desc) & ALT_MMU_TTB2_LARGE_PAGE_B_MASK) >> 2)
#define ALT_MMU_TTB2_LARGE_PAGE_B_SET(val)                (((val) << 2) & ALT_MMU_TTB2_LARGE_PAGE_B_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [C]
 *
 * The [C] field of the memory region attributes. [C] is an arcane reference to
 * Cacheable attribute.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_C_MASK                    0x00000008
#define ALT_MMU_TTB2_LARGE_PAGE_C_GET(desc)               (((desc) & ALT_MMU_TTB2_LARGE_PAGE_C_MASK) >> 3)
#define ALT_MMU_TTB2_LARGE_PAGE_C_SET(val)                (((val) << 3) & ALT_MMU_TTB2_LARGE_PAGE_C_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [AP]
 *
 * Access Permissions bits.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_AP_MASK                   0x00000230
#define ALT_MMU_TTB2_LARGE_PAGE_AP_GET(desc)              ((((desc) & 0x00000200) >> 7) | (((desc) & 0x00000030) >> 4))
#define ALT_MMU_TTB2_LARGE_PAGE_AP_SET(val)               ((((val) << 7) & 0x00000200) | (((val) << 4) & 0x00000030))
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [S]
 *
 * The Shareable bit. Determines whether the addressed region is shareable memory.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_S_MASK                    0x00000400
#define ALT_MMU_TTB2_LARGE_PAGE_S_GET(desc)               (((desc) & ALT_MMU_TTB2_LARGE_PAGE_S_MASK) >> 10)
#define ALT_MMU_TTB2_LARGE_PAGE_S_SET(val)                (((val) << 10) & ALT_MMU_TTB2_LARGE_PAGE_S_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [nG]
 *
 * The not global bit. Determines how the translation is marked in the TLB.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_NG_MASK                   0x00000800
#define ALT_MMU_TTB2_LARGE_PAGE_NG_GET(desc)              (((desc) & ALT_MMU_TTB2_LARGE_PAGE_NG_MASK) >> 11)
#define ALT_MMU_TTB2_LARGE_PAGE_NG_SET(val)               (((val) << 11) & ALT_MMU_TTB2_LARGE_PAGE_NG_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [TEX]
 *
 * The [TEX] field of the memory region attributes. [TEX] is an arcane reference to
 * Type EXtension attribute.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_TEX_MASK                  0x00007000
#define ALT_MMU_TTB2_LARGE_PAGE_TEX_GET(desc)             (((desc) & ALT_MMU_TTB2_LARGE_PAGE_TEX_MASK) >> 12)
#define ALT_MMU_TTB2_LARGE_PAGE_TEX_SET(val)              (((val) << 12) & ALT_MMU_TTB2_LARGE_PAGE_TEX_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry [XN]
 *
 * The Execute-Never bit. Determines whether the processor can execute software
 * from the addressed region.
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_XN_MASK                   0x00008000
#define ALT_MMU_TTB2_LARGE_PAGE_XN_GET(desc)              (((desc) & ALT_MMU_TTB2_LARGE_PAGE_XN_MASK) >> 15)
#define ALT_MMU_TTB2_LARGE_PAGE_XN_SET(val)               (((val) << 15) & ALT_MMU_TTB2_LARGE_PAGE_XN_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Large Page Table Entry Large Page Base Address
 * @{
 */
#define ALT_MMU_TTB2_LARGE_PAGE_BASE_ADDR_MASK            0xffff0000
#define ALT_MMU_TTB2_LARGE_PAGE_BASE_ADDR_GET(desc)       (((desc) & ALT_MMU_TTB2_LARGE_PAGE_BASE_ADDR_MASK) >> 16)
#define ALT_MMU_TTB2_LARGE_PAGE_BASE_ADDR_SET(val)        (((val) << 16) & ALT_MMU_TTB2_LARGE_PAGE_BASE_ADDR_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [XN]
 *
 * The Execute-Never bit. Determines whether the processor can execute software
 * from the addressed region.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_XN_MASK                   0x00000001
#define ALT_MMU_TTB2_SMALL_PAGE_XN_GET(desc)              (((desc) & ALT_MMU_TTB2_SMALL_PAGE_XN_MASK) >> 0)
#define ALT_MMU_TTB2_SMALL_PAGE_XN_SET(val)               (((val) << 0) & ALT_MMU_TTB2_SMALL_PAGE_XN_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [B]
 *
 * The [B] field of the memory region attributes. [B] is an arcane reference to
 * Bufferable attribute.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_B_MASK                    0x00000004
#define ALT_MMU_TTB2_SMALL_PAGE_B_GET(desc)               (((desc) & ALT_MMU_TTB2_SMALL_PAGE_B_MASK) >> 2)
#define ALT_MMU_TTB2_SMALL_PAGE_B_SET(val)                (((val) << 2) & ALT_MMU_TTB2_SMALL_PAGE_B_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [C]
 *
 * The [C] field of the memory region attributes. [C] is an arcane reference to
 * Cacheable attribute.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_C_MASK                    0x00000008
#define ALT_MMU_TTB2_SMALL_PAGE_C_GET(desc)               (((desc) & ALT_MMU_TTB2_SMALL_PAGE_C_MASK) >> 3)
#define ALT_MMU_TTB2_SMALL_PAGE_C_SET(val)                (((val) << 3) & ALT_MMU_TTB2_SMALL_PAGE_C_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [AP]
 *
 * Access Permissions bits.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_AP_MASK                   0x00000230
#define ALT_MMU_TTB2_SMALL_PAGE_AP_GET(desc)              ((((desc) & 0x00000200) >> 7) | (((desc) & 0x00000030) >> 4))
#define ALT_MMU_TTB2_SMALL_PAGE_AP_SET(val)               ((((val) << 7) & 0x00000200) | (((val) << 4) & 0x00000030))
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [TEX]
 *
 * The [TEX] field of the memory region attributes. [TEX] is an arcane reference to
 * Type EXtension attribute.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_TEX_MASK                  0x000001c0
#define ALT_MMU_TTB2_SMALL_PAGE_TEX_GET(desc)             (((desc) & ALT_MMU_TTB2_SMALL_PAGE_TEX_MASK) >> 6)
#define ALT_MMU_TTB2_SMALL_PAGE_TEX_SET(val)              (((val) << 6) & ALT_MMU_TTB2_SMALL_PAGE_TEX_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [S]
 *
 * The Shareable bit. Determines whether the addressed region is shareable memory.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_S_MASK                    0x00000400
#define ALT_MMU_TTB2_SMALL_PAGE_S_GET(desc)               (((desc) & ALT_MMU_TTB2_SMALL_PAGE_S_MASK) >> 10)
#define ALT_MMU_TTB2_SMALL_PAGE_S_SET(val)                (((val) << 10) & ALT_MMU_TTB2_SMALL_PAGE_S_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry [nG]
 *
 * The not global bit. Determines how the translation is marked in the TLB.
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_NG_MASK                   0x00000800
#define ALT_MMU_TTB2_SMALL_PAGE_NG_GET(desc)              (((desc) & ALT_MMU_TTB2_SMALL_PAGE_NG_MASK) >> 11)
#define ALT_MMU_TTB2_SMALL_PAGE_NG_SET(val)               (((val) << 11) & ALT_MMU_TTB2_SMALL_PAGE_NG_MASK)
/*! @} */

/*!
 * \name Second Level Translation Table Small Page Table Entry Large Page Base Address
 * @{
 */
#define ALT_MMU_TTB2_SMALL_PAGE_BASE_ADDR_MASK            0xfffff000
#define ALT_MMU_TTB2_SMALL_PAGE_BASE_ADDR_GET(desc)       (((desc) & ALT_MMU_TTB2_SMALL_PAGE_BASE_ADDR_MASK) >> 12)
#define ALT_MMU_TTB2_SMALL_PAGE_BASE_ADDR_SET(val)        (((val) << 12) & ALT_MMU_TTB2_SMALL_PAGE_BASE_ADDR_MASK)
/*! @} */

/*! @} */


/******************************************************************************/
/*! \addtogroup ALT_MMU_MGMT_STRUCT_TTB1 MMU Management Data Structures - First Level Translation Table
 *
 * The data structure declarations in this section support direct access to the
 * short-descriptor first-level table entries and their constituent fields.
 *
 * These data structures are an alternative method to create descriptor entry
 * values that are passed to a first level translation table contruction function
 * such as alt_mmu_ttb1_desc_set().
 *
 * @{
 */

/*!
 * This type defines the structure of a First Level Translation Table Fault Entry.
 */
typedef struct ALT_MMU_TTB1_FAULT_s
{
    uint32_t    type        :  2;
    uint32_t                : 30;
} ALT_MMU_TTB1_FAULT_t;

/*!
 * This type defines a union for accessing a First Level Translation Table Fault
 * Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB1_FAULT_ENTRY_u
{
    ALT_MMU_TTB1_FAULT_t        fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB1_FAULT_ENTRY_t;

/*!
 * This type defines the structure of a First Level Translation Table Page Table
 * Entry.
 */
typedef struct ALT_MMU_TTB1_PAGE_TABLE_s
{
    uint32_t    type        :  2;   /*!< Descriptor type field */
    uint32_t                :  1;
    uint32_t    ns          :  1;   /*!< The Non-Secure [NS] bit. This bit specifies
                                     *   whether the translated PA is in the Secure
                                     *   or Non-Secure address map.
                                     */
    uint32_t                :  1;
    uint32_t    domain      :  4;   /*!< Domain field. Page table descriptor applies
                                     *   to all entries in the corresponding
                                     *   second-level translation table.
                                     */
    uint32_t                :  1;
    uint32_t    base_addr   : 22;   /*!< Page Table Base Address */
} ALT_MMU_TTB1_PAGE_TABLE_t;

/*!
 * This type defines a union for accessing a First Level Translation Table Page
 * Table Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB1_PAGE_TABLE_ENTRY_u
{
    ALT_MMU_TTB1_PAGE_TABLE_t   fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB1_PAGE_TABLE_ENTRY_t;

/*!
 * This type defines the structure of a First Level Translation Table Section Entry.
 */
typedef struct ALT_MMU_TTB1_SECTION_s
{
    uint32_t    type        :  2;   /*!< Descriptor type field */
    uint32_t    b           :  1;   /*!< The [B] field of the memory region
                                     *   attributes. [B] is an arcane reference to
                                     *   Bufferable attribute.
                                     */
    uint32_t    c           :  1;   /*!< The [C] field of the memory region
                                     *   attributes. [C] is an arcane reference to
                                     *   Cacheable attribute.
                                     */

    uint32_t    xn          :  1;   /*!< The Execute-Never bit. Determines whether
                                     *   the processor can execute software from the
                                     *   addressed region.
                                     */
    uint32_t    domain      :  4;   /*!< Domain field. */
    uint32_t                :  1;
    uint32_t    ap_1_0      :  2;   /*!< Access Permissions AP[1:0] bits. */
    uint32_t    tex         :  3;   /*!< The [TEX] field of the memory region
                                     *   attributes. [TEX] is an arcane reference to
                                     *   Type EXtension attribute.
                                     */
    uint32_t    ap_2        :  1;   /*!< Access Permissions AP[2] bits. */
    uint32_t    s           :  1;   /*!< The Shareable bit. Determines whether the
                                     *   addressed region is shareable memory.
                                     */
    uint32_t    ng          :  1;   /*!< The not global bit. Determines how the
                                     *   translation is marked in the TLB.
                                     */
    uint32_t                :  1;
    uint32_t    ns          :  1;   /*!< The Non-Secure [NS] bit. This bit specifies
                                     *   whether the translated PA is in the Secure
                                     *   or Non-Secure address map.
                                     */
    uint32_t    base_addr   : 12;   /*!< Section Base Address */
} ALT_MMU_TTB1_SECTION_t;

/*!
 * This type defines a union for accessing a First Level Translation Table Section
 * Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB1_SECTION_ENTRY_u
{
    ALT_MMU_TTB1_SECTION_t      fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB1_SECTION_ENTRY_t;

/*!
 * This type defines the structure of a First Level Translation Table Supersection
 * Entry.
 */
typedef struct ALT_MMU_TTB1_SUPERSECTION_s
{
    uint32_t    type        :  2;   /*!< Descriptor type field */
    uint32_t    b           :  1;   /*!< The [B] field of the memory region
                                     *   attributes. [B] is an arcane reference to
                                     *   Bufferable attribute.
                                     */
    uint32_t    c           :  1;   /*!< The [C] field of the memory region
                                     *   attributes. [C] is an arcane reference to
                                     *   Cacheable attribute.
                                     */

    uint32_t    xn          :  1;   /*!< The Execute-Never bit. Determines whether
                                     *   the processor can execute software from the
                                     *   addressed region.
                                     */
    uint32_t    ext_base_addr:  4;   /*!< Extended Base Address (Bits 39:36). */
    uint32_t                :  1;
    uint32_t    ap_1_0      :  2;   /*!< Access Permissions AP[1:0] bits. */
    uint32_t    tex         :  3;   /*!< The [TEX] field of the memory region
                                     *   attributes. [TEX] is an arcane reference to
                                     *   Type EXtension attribute.
                                     */
    uint32_t    ap_2        :  1;   /*!< Access Permissions AP[2] bits. */
    uint32_t    s           :  1;   /*!< The Shareable bit. Determines whether the
                                     *   addressed region is shareable memory.
                                     */
    uint32_t    ng          :  1;   /*!< The not global bit. Determines how the
                                     *   translation is marked in the TLB.
                                     */
    uint32_t                :  1;
    uint32_t    ns          :  1;   /*!< The Non-Secure [NS] bit. This bit specifies
                                     *   whether the translated PA is in the Secure
                                     *   or Non-Secure address map.
                                     */
    uint32_t                :  4;
    uint32_t    base_addr   :  8;   /*!< Supersection Base Address */
} ALT_MMU_TTB1_SUPERSECTION_t;

/*!
 * This type defines a union for accessing a First Level Translation Table
 * Supersection Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB1_SUPERSECTION_ENTRY_u
{
    ALT_MMU_TTB1_SUPERSECTION_t fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB1_SUPERSECTION_ENTRY_t;

/*! @} */

/******************************************************************************/
/*! \addtogroup ALT_MMU_MGMT_STRUCT_TTB2 MMU Management Data Structures - Second Level Translation Table
 *
 * The data structure declarations in this section support direct access to the
 * short-descriptor second-level table entries and their constituent fields.
 *
 * These data structures are an alternative method to create descriptor entry
 * values that are passed to a first level translation table contruction function
 * such as alt_mmu_ttb2_desc_set().
 *
 * @{
 */

/*!
 * This type defines the structure of a Second Level Translation Table Fault Entry.
 */
typedef struct ALT_MMU_TTB2_FAULT_s
{
    uint32_t    type        :  2;       /* b00 */
    uint32_t                : 30;       /* IGNORE */
} ALT_MMU_TTB2_FAULT_t;

/*!
 * This type defines a union for accessing a Second Level Translation Table Fault
 * Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB2_FAULT_ENTRY_u
{
    ALT_MMU_TTB2_FAULT_t        fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB2_FAULT_ENTRY_t;

/*!
 * This type defines the structure of a Second Level Translation Table Large Page
 * Table Entry.
 */
typedef struct ALT_MMU_TTB2_LARGE_PAGE_s
{
    uint32_t                :  2;   /*!< always b01 */
    uint32_t    b           :  1;   /*!< The [B] field of the memory region
                                     *   attributes. [B] is an arcane reference to
                                     *   Bufferable attribute.
                                     */
    uint32_t    c           :  1;   /*!< The [C] field of the memory region
                                     *   attributes. [C] is an arcane reference to
                                     *   Cacheable attribute.
                                     */
    uint32_t    ap_1_0      :  2;   /*!< Access Permissions AP[1:0] bits. */
    uint32_t                :  3;       /* SBZ - b000 */
    uint32_t    ap_2        :  1;   /*!< Access Permissions AP[2] bits. */
    uint32_t    s           :  1;   /*!< The Shareable bit. Determines whether the
                                     *   addressed region is shareable memory.
                                     */
    uint32_t    ng          :  1;   /*!< The not global bit. Determines how the
                                     *   translation is marked in the TLB.
                                     */
    uint32_t    tex         :  3;   /*!< The [TEX] field of the memory region
                                     *   attributes. [TEX] is an arcane reference to
                                     *   Type EXtension attribute.
                                     */
    uint32_t    xn          :  1;   /*!< The Execute-Never bit. Determines whether
                                     *   the processor can execute software from the
                                     *   addressed region.
                                     */
    uint32_t    base_addr   : 16;   /*!< Large Page Base Address PA[31:16] */
} ALT_MMU_TTB2_LARGE_PAGE_t;

/*!
 * This type defines a union for accessing a Second Level Translation Table Large
 * Page Table Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB2_LARGE_PAGE_ENTRY_u
{
    ALT_MMU_TTB2_LARGE_PAGE_t   fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB2_LARGE_PAGE_ENTRY_t;

/*!
 * This type defines the structure of a Second Level Translation Table Small Page
 * Table Entry.
 */
typedef struct ALT_MMU_TTB2_SMALL_PAGE_s
{
    uint32_t    xn          :  1;   /*!< The Execute-Never bit. Determines whether
                                     *   the processor can execute software from the
                                     *   addressed region.
                                     */
    uint32_t                :  1;   /*!< always b1 */
    uint32_t    b           :  1;   /*!< The [B] field of the memory region
                                     *   attributes. [B] is an arcane reference to
                                     *   Bufferable attribute.
                                     */
    uint32_t    c           :  1;   /*!< The [C] field of the memory region
                                     *   attributes. [C] is an arcane reference to
                                     *   Cacheable attribute.
                                     */
    uint32_t    ap_1_0      :  2;   /*!< Access Permissions AP[1:0] bits. */
    uint32_t    tex         :  3;   /*!< The [TEX] field of the memory region
                                     *   attributes. [TEX] is an arcane reference to
                                     *   Type EXtension attribute.
                                     */
    uint32_t    ap_2        :  1;   /*!< Access Permissions AP[2] bits. */
    uint32_t    s           :  1;   /*!< The Shareable bit. Determines whether the
                                     *   addressed region is shareable memory.
                                     */
    uint32_t    ng          :  1;   /*!< The not global bit. Determines how the
                                     *   translation is marked in the TLB.
                                     */
    uint32_t    base_addr   : 20;   /*!< Small Page Base Address PA[31:12] */
} ALT_MMU_TTB2_SMALL_PAGE_t;

/*!
 * This type defines a union for accessing a Second Level Translation Table Small
 * Page Table Entry by fields or aggregate raw entry value.
 */
typedef union ALT_MMU_TTB2_SMALL_PAGE_ENTRY_u
{
    ALT_MMU_TTB2_SMALL_PAGE_t   fld;    /*!< access to individual entry data fields */
    uint32_t                    raw;    /*!< access to aggregate entry value */
} ALT_MMU_TTB2_SMALL_PAGE_ENTRY_t;

/*! @} */

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __ALT_MMU_H__ */
