PANASONIC AC INTERFACE rev G - FABRICATION NOTES
=================================================

Board name      : panasonic-p11-revG
Size            : 48.5 x 18.0 mm
Layers          : 2
Material        : FR-4, TG150
Thickness       : 1.0 mm
Copper weight   : 1 oz outer
Surface finish  : Lead-free HASL (ENIG acceptable)
Solder mask     : Green
Silkscreen      : White, both sides
Min trace/space : 0.20 mm / 0.153 mm
Min drill       : 0.30 mm
Via process     : Tented
Impedance       : None specified
Quantity        : 10

INTENTIONAL OUTLINE FEATURES - PLEASE DO NOT QUERY
--------------------------------------------------
1. NOTCH. A 3.0 mm deep x 6.0 mm wide rectangular cut-out is present in the
   right board edge, centred on the 1.27 mm connector (J11). This is an
   intentional profile feature, not an error. Please route it with the board
   profile. Internal corner radii per your standard router bit are acceptable.

2. CHAMFERS. All four outer corners are chamfered 1.5 mm at 45 degrees.
   This is intentional.

3. Copper-to-edge clearance is 0.38 mm at the chamfered corners and 0.40 mm
   at the notch. This matches a previously fabricated revision of this board
   and is accepted by the designer.

NOTES
-----
- All components are through-hole. No SMD assembly required.
- Two 2.5 mm non-plated holes (near the notch) are cable-tie holes and must
  remain unplated.
- No panelisation required at this size; single pieces are fine.

Contact the buyer before making any change to the board outline.

FILES
-----
panasonic-p11-revG-F_Cu.gtl          Top copper
panasonic-p11-revG-B_Cu.gbl          Bottom copper
panasonic-p11-revG-F_Mask.gts        Top solder mask
panasonic-p11-revG-B_Mask.gbs        Bottom solder mask
panasonic-p11-revG-F_Silkscreen.gto  Top silkscreen
panasonic-p11-revG-B_Silkscreen.gbo  Bottom silkscreen
panasonic-p11-revG-Edge_Cuts.gm1     Board outline
panasonic-p11-revG-PTH.drl           Plated holes (Excellon)
panasonic-p11-revG-NPTH.drl          Non-plated holes (Excellon)
panasonic-p11-revG-job.gbrjob        Gerber job file
