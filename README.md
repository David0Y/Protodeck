# Protodeck (3d-prints branch)

Archive of the .stl and .3mf 3D-printable enclosure models created during the iterative mechanical design process of the Protodeck casing, culminating in the final modular housing for the 40 × 60 mm perfboard, STM32F411CE BlackPill, and dual nRF24L01+PA+LNA transceivers.

# Purpose & Scope

**Iterative Design History:** 
Preserves intermediate enclosure prototypes and tolerance test revisions produced while refining the perfboard slide-in slot, transceiver cutouts, and modular ceiling/wall fitment. 

**Final Printable Release:** 
Provides the final multi-part casing files ready for FDM 3D printing (protocase_final, protocase_ceiling, and protocase_wall, along with final.3mf).

**Final Enclosure Parts**:
To print the finished enclosure, use either the individual .stl files or the pre-configured .3mf project file (adjusted to your specific printer and filament):

**protocase_8_4.stl:** Main enclosure body housing the 40 × 60 mm perfboard slot, BlackPill USB-C/debugger openings, and dual transceiver pockets.

**protocase_ceiling_10.stl:** Top cover with cutouts for the exposed 4-pin ST-Link debugger header.

**protocase_wall_3.stl:** Locking end wall that secures the perfboard inside the slot and prevents board movement when plugging in the USB-C cable.

**protocase_final.3mf** Combined slicer project reference file.  

For more details look into the **hardware bringup document** and into the **casing design log document** under the main branch.
