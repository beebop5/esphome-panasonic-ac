#!/usr/bin/env python3
"""Generate panasonic-p11 rev H placement (geometry from the fabbed gerbers)."""
sc=[0]
def T(): sc[0]+=1; return "bb000000-0000-0000-0000-%012d"%sc[0]

NN={0:"",1:"GND",2:"+5V",3:"AC_5V",4:"+3V3",5:"ESP_TX",6:"ESP_RX",7:"AC_TX_5V",
    8:"AC_RX_5V",13:"IO1",14:"IO0",15:"IO10",16:"IO7",17:"IO10_5V",18:"IO7_5V"}
def nr(n): return ' (net %d "%s")'%(n,NN[n]) if n else ''

LAYERS=["F.Cu signal","B.Cu signal"]
LAYSTR='''  (layers
    (0 "F.Cu" signal)
    (31 "B.Cu" signal)
    (32 "B.Adhes" user "B.Adhesive")
    (33 "F.Adhes" user "F.Adhesive")
    (34 "B.Paste" user)
    (35 "F.Paste" user)
    (36 "B.SilkS" user "B.Silkscreen")
    (37 "F.SilkS" user "F.Silkscreen")
    (38 "B.Mask" user)
    (39 "F.Mask" user)
    (40 "Dwgs.User" user "User.Drawings")
    (41 "Cmts.User" user "User.Comments")
    (42 "Eco1.User" user "User.Eco1")
    (43 "Eco2.User" user "User.Eco2")
    (44 "Edge.Cuts" user)
    (45 "Margin" user)
    (46 "B.CrtYd" user "B.Courtyard")
    (47 "F.CrtYd" user "F.Courtyard")
    (48 "B.Fab" user)
    (49 "F.Fab" user)
  )'''

def th(num,x,y,net,shape="circle",size=1.7,drill=1.0):
    return ('    (pad "%s" thru_hole %s (at %g %g) (size %g %g) (drill %g) '
            '(layers "*.Cu" "*.Mask")%s (tstamp %s))\n')%(num,shape,x,y,size,size,drill,nr(net),T())
def ft(k,t,x,y,sz,lay="F.SilkS",rot=None):
    at = "%g %g"%(x,y) if rot is None else "%g %g %g"%(x,y,rot)
    return ('    (fp_text %s "%s" (at %s) (layer "%s") (effects (font (size %g %g) '
            '(thickness 0.12))) (tstamp %s))\n')%(k,t,at,lay,sz,sz,T())
def box(hw,hh):
    o=''; pts=[(-hw,-hh),(hw,-hh),(hw,hh),(-hw,hh)]
    for a,b in zip(pts,pts[1:]+[pts[0]]):
        o+=('    (fp_line (start %g %g) (end %g %g) (stroke (width 0.12) (type solid)) '
            '(layer "F.SilkS") (tstamp %s))\n')%(a[0],a[1],b[0],b[1],T())
    return o
SILK=[]
def lbl(t,x,y,sz=0.42,rot=0):
    SILK.append('  (gr_text "%s" (at %g %g %g) (layer "F.SilkS") (tstamp %s) '
                '(effects (font (size %g %g) (thickness 0.09))))\n'%(t,x,y,rot,T(),sz,sz))

s='(kicad_pcb (version 20221018) (generator pcbnew)\n  (general (thickness 1.6))\n  (paper "A4")\n'
s+=LAYSTR+'\n  (setup (pad_to_mask_clearance 0.05)\n    (pcbplotparams (usegerberattributes true) '
s+='(creategerberjobfile true) (svgprecision 6) (mode 1) (drillshape 1) (scaleselection 1) (outputdirectory "gerbers/")))\n'
for i in sorted(NN): s+='  (net %d "%s")\n'%(i,NN[i])
s+='\n'
for x1,y1,x2,y2 in [(21.5,23,69,23),(69,23,70.5,24.5),(70.5,24.5,70.5,29),(70.5,29,67.5,29),
    (67.5,29,67.5,35),(67.5,35,70.5,35),(70.5,35,70.5,39.5),(70.5,39.5,69,41),
    (69,41,21.5,41),(21.5,41,20,39.5),(20,39.5,20,24.5),(20,24.5,21.5,23)]:
    s+='  (gr_line (start %g %g) (end %g %g) (layer "Edge.Cuts") (width 0.1) (tstamp %s))\n'%(x1,y1,x2,y2,T())

# ---------------- U1 ESP32-C3 SuperMini ----------------
UX=[22.36+2.54*i for i in range(8)]
TOPN=[2,1,4,6,5,None,13,14]; TOPL=["5V","G","3V3","IO4","IO3","IO2","IO1","IO0"]
BOTN=[None,None,16,None,None,15,None,None]; BOTL=["IO5","IO6","IO7","IO8","IO9","IO10","IO20","IO21"]
s+='  (footprint "local:U1" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000001) (at 31.25 32) (attr through_hole)\n'
s+=ft("reference","ESP32-C3",0,0,0.9)+ft("value","ESP32-C3-SuperMini",0,1.3,0.65,"F.Fab")
for i in range(8): s+=th(str(i+1),UX[i]-31.25,24.38-32,TOPN[i],shape=("rect" if i==0 else "circle"))
for i in range(8): s+=th(str(i+9),UX[i]-31.25,39.62-32,BOTN[i])
s+=box(11.25,9)+'  )\n'
for i in range(8):
    lbl(TOPL[i],UX[i],25.75,0.42); lbl(BOTL[i],UX[i],38.25,0.42)

# ---------------- U2 level converter (single 10.16) ----------------
U2Y=[24.38+2.54*i for i in range(6)]; cy=(U2Y[0]+U2Y[5])/2.0
LVN=[5,15,4,1,16,6];  LVL=["TXI","RXO","LV","G","RXO","TXI"]
HVN=[8,17,2,1,18,7];  HVL=["TXO","RXI","HV","G","RXI","TXO"]
s+='  (footprint "local:U2" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000002) (at 51.0 %g) (attr through_hole)\n'%cy
s+=ft("reference","BSS138",0,0,0.9)+ft("value","BSS138-2ch-module",0,1.25,0.6,"F.Fab")
for i in range(6): s+=th("L%d"%(i+1),45.92-51.0,U2Y[i]-cy,LVN[i],shape=("rect" if i==0 else "circle"))
for i in range(6): s+=th("H%d"%(i+1),56.08-51.0,U2Y[i]-cy,HVN[i])
s+=box(6.3,7.2)+'  )\n'
for i in range(6): lbl(LVL[i],47.6,U2Y[i],0.38); lbl(HVL[i],54.5,U2Y[i],0.38)

# ---------------- D1 (bottom strip) ----------------
s+='  (footprint "local:D1" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000004) (at 51.7 39.6) (attr through_hole)\n'
s+=ft("reference","D1",0,-1.75,0.55)+ft("value","1N5819_DO41",0,1.75,0.5,"F.Fab")
s+=th("1",-4.445,0,2,shape="rect",size=1.6,drill=0.9)+th("2",4.445,0,3,size=1.6,drill=0.9)
for a,b in [((-2.35,-1.35),(2.35,-1.35)),((2.35,-1.35),(2.35,1.35)),((2.35,1.35),(-2.35,1.35)),((-2.35,1.35),(-2.35,-1.35))]:
    s+='    (fp_line (start %g %g) (end %g %g) (stroke (width 0.12) (type solid)) (layer "F.SilkS") (tstamp %s))\n'%(a[0],a[1],b[0],b[1],T())
s+='    (fp_line (start -1.5 -1.35) (end -1.5 1.35) (stroke (width 0.15) (type solid)) (layer "F.SilkS") (tstamp %s))\n'%T()
s+='  )\n'
lbl("K",45.7,38.3,0.4); lbl("A",57.4,38.3,0.4)

# ---------------- C1 between converter and AC headers, body lies flat vertically -----
CX,CY=59.1,27.6
s+='  (footprint "local:C1" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000005) (at %g %g) (attr through_hole)\n'%(CX,CY)
s+=ft("reference","C1",1.9,-1.3,0.55)+ft("value","100uF_16V",0,-2.4,0.5,"F.Fab")
s+=th("1",-1.0,0,2,shape="rect",size=1.6,drill=0.9)+th("2",1.0,0,1,size=1.6,drill=0.9)
# can body 4.5 x 11 mm lying flat, extending downward from the leads
for a,b in [((-2.0,0.9),(2.0,0.9)),((2.0,0.9),(2.0,8.9)),((2.0,8.9),(-2.0,8.9)),((-2.0,8.9),(-2.0,0.9))]:
    s+='    (fp_line (start %g %g) (end %g %g) (stroke (width 0.1) (type solid)) (layer "F.SilkS") (tstamp %s))\n'%(a[0],a[1],b[0],b[1],T())
s+='  )\n'
lbl("+",58.1,26.25,0.5); lbl("-",60.1,26.25,0.5)

# ---------------- J1 CN-WLAN 1.27mm (as fabbed) ----------------
J1Y=[36.445,35.175,33.905,32.635,31.365,30.095,28.825,27.555]; J1N={1:3,2:7,3:8,8:1}
s+='  (footprint "local:J11" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000003) (at 66.5 32) (attr through_hole)\n'
s+=ft("reference","J11",-0.6,-5.7,0.55)+ft("value","1.27_RA_8way",0,5.7,0.5,"F.Fab")
for i in range(8):
    s+=('    (pad "%d" thru_hole %s (at 0 %g) (size 1.2 1.0) (drill 0.65) (layers "*.Cu" "*.Mask")%s (tstamp %s))\n'
        )%(i+1,("rect" if i==0 else "oval"),J1Y[i]-32,nr(J1N.get(i+1)),T())
s+='  )\n'
for i,t in enumerate(["1","2","3","4","5","6","7","8"]): lbl(t,64.95,J1Y[i],0.34)

# ---------------- J4 CN-CNT 5p JST-PA 2.0mm (pin4 = +12V, isolated) -------
J4N=[3,7,8,None,1]; J4L=["5V","TX","RX","NC","G"]
s+='  (footprint "local:CNT" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-000000000015) (at 63.2 32) (attr through_hole)\n'
s+=ft("reference","CNT",-1.3,0,0.5,rot=90)+ft("value","CN-CNT_5p_2.54",0,6.9,0.5,"F.Fab")
for i in range(5):
    s+=th(str(i+1),0,-5.08+2.54*i,J4N[i],shape=("rect" if i==0 else "circle"),size=1.7,drill=1.0)
    lbl(J4L[i],61.35,32-5.08+2.54*i,0.32)
s+='  )\n'

# ---------------- J2 top header (3V3 side) / J3 bottom header (5V side) ----
def hdr(ref,tag,cx,cy,nets,labels,laby):
    o='  (footprint "local:%s" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-0000000000%02d) (at %g %g) (attr through_hole)\n'%(ref,tag,cx,cy)
    refdx,refdy=((-0.1,1.8) if ref=="J1" else (-5.3,1.05))
    o+='    (fp_text reference "%s" (at %g %g) (layer "F.SilkS") (effects (font (size 0.5 0.5) (thickness 0.09))) (tstamp %s))\n'%(ref,refdx,refdy,T())
    o+='    (fp_text value "1x4" (at 0 0) (layer "F.Fab") (effects (font (size 0.45 0.45) (thickness 0.09))) (tstamp %s))\n'%T()
    for i,(n,l) in enumerate(zip(nets,labels)):
        px=-3.81+2.54*i
        o+=('    (pad "%d" thru_hole %s (at %g 0) (size 1.7 1.7) (drill 1.0) (layers "*.Cu" "*.Mask")%s (tstamp %s))\n'
            )%(i+1,("rect" if i==0 else "circle"),px,nr(n),T())
        lbl(l,cx+px,laby,0.36)
    o+='  )\n'; return o
s+=hdr("J1",11,62.0,24.3,[13,14,1,4],["IO1","IO0","G","3V3"],25.65)
s+=hdr("J2",12,62.0,39.7,[17,18,1,2],["IO10","IO7","G","5V"],38.35)

# ---------------- cable-tie holes ----------------
for ref,tag,x,y in [("H1",13,68.3,25.6),("H2",14,68.3,38.4)]:
    s+='  (footprint "local:%s" (layer "F.Cu") (tstamp bb000000-1111-0000-0000-0000000000%02d) (at %g %g) (attr through_hole)\n'%(ref,tag,x,y)
    # reference/value kept but hidden - no silk clutter on the tie holes
    s+='    (fp_text reference "%s" (at 0 0) (layer "F.Fab") hide (effects (font (size 0.5 0.5) (thickness 0.09))) (tstamp %s))\n'%(ref,T())
    s+='    (fp_text value "tie" (at 0 0) (layer "F.Fab") hide (effects (font (size 0.5 0.5) (thickness 0.09))) (tstamp %s))\n'%T()
    s+='    (pad "" np_thru_hole circle (at 0 0) (size 2.5 2.5) (drill 2.5) (layers "F&B.Cu" "*.Mask") (tstamp %s))\n  )\n'%T()


for _t,_x,_y,_s in [("PANASONIC AC IF  rev H",31.8,27.3,1.1),
                    ("CNT PIN4=12V NO CONNECT",31.8,30.2,1.0),
                    ("github.com/bl0ckstat/",31.8,33.1,1.0),
                    ("esphome-panasonic-ac",31.8,35.7,1.0)]:
    SILK.append('  (gr_text "%s" (at %g %g 0) (layer "B.SilkS") (tstamp %s) (effects (font (size %g %g) (thickness 0.18)) (justify mirror)))\n'%(_t,_x,_y,T(),_s,_s))
s+=''.join(SILK)
s+='''  (zone (net 1) (net_name "GND") (layers "B.Cu") (name "gnd_pour") (hatch edge 0.5) (tstamp 535fb199-6ba4-492e-a39b-b3523e7e0169)
    (connect_pads (clearance 0.3)) (min_thickness 0.25) (filled_areas_thickness no)
    (fill yes (thermal_gap 0.35) (thermal_bridge_width 0.6))
    (polygon (pts (xy 19 22) (xy 71.5 22) (xy 71.5 42) (xy 19 42))))
  (zone (net 0) (net_name "") (layers "F.Cu" "B.Cu") (name "antenna_keepout") (hatch edge 0.5) (tstamp 8d6939db-2ceb-4ed6-99d0-f7b9e3291b5a)
    (connect_pads (clearance 0.3)) (min_thickness 0.25) (filled_areas_thickness no)
    (keepout (tracks allowed) (vias not_allowed) (pads allowed) (copperpour not_allowed) (footprints allowed))
    (polygon (pts (xy 40.7 23) (xy 41.9 23) (xy 41.9 41) (xy 40.7 41))))
)
'''
open("panasonic-p11-revG.kicad_pcb","w").write(s)
print("rev H placement written")

# NOTE: J2 reference is nudged +1.05mm Y in the final board to clear D1's 'A' label.
