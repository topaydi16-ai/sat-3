#set page(
  paper: "a4",
  margin: (top: 1.4cm, bottom: 1.4cm, left: 1.8cm, right: 1.8cm)
)

#set text(
  font: "Times New Roman",
  size: 9.6pt,
  lang: "en"
)

#set par(justify: true, leading: 0.52em)

#align(center)[
  #text(8.5pt, fill: luma(100), tracking: 2pt)[WOLFRAM RULE 30 PRIZE RESEARCH | AUTONOMOUS MATHEMATICAL DISCOVERY] \
  #v(0.1cm)
  #text(16pt, weight: "bold")[THE UNIVERSAL BOUNDARY ANNIHILATION THEOREM] \
  #v(0.1cm)
  #text(10.5pt, style: "italic", fill: rgb("#003366"))[Exact Collapse and Annihilation of Left Diagonals in Elementary Cellular Automaton Rule 30] \
  #v(0.15cm)
  #line(length: 100%, stroke: 0.5pt + luma(150))
]

#v(0.15cm)

#text(weight: "bold")[The Setting & Rowland's Open Bound:] \
Stephen Wolfram's Rule 30 is defined by $c(t+1, x) = c(t, x-1) xor (c(t, x) or c(t, x+1))$. \
Its left diagonals $D_j(t) = c(t, j - t)$ follow the Rowland-Wolfram recurrence:
$ D_(j+2)(t+1) = D_j(t) xor (D_(j+1)(t) or D_(j+2)(t)) $
The Rowland (2006) upper bound states that diagonal $j$ has period dividing $2^j$ ($2^5 = 32$, $2^7 = 128$). We replace this loose existential bound with two exact closed-form collapse theorems:

#v(0.15cm)

#rect(fill: rgb("#f4f7fb"), stroke: 1.2pt + rgb("#003366"), width: 100%, inset: 8pt, radius: 3pt)[
  #text(10.5pt, weight: "bold", fill: rgb("#003366"))[Master Ladder Theorems (Lean 4 Certified, Zero Custom Axioms)] \
  #v(0.05cm)
  #grid(
    columns: (1fr, 1fr),
    gutter: 10pt,
    [*(I) Diagonal 5 Collapse:* \ $bold(D_5(t) = D_3(t)) quad (forall t >= 5)$ \ Exact period 2, shattering $2^5 = 32$.],
    [*(II) Diagonal 7 Annihilation ($z_2 = 7$):* \ $bold(D_7(t) = 0) quad (forall t >= 7)$ \ Identically zero, shattering $2^7 = 128$.]
  )
]

#v(0.15cm)

#text(10.5pt, weight: "bold")[The Complete Constructive Proof:]

#v(0.05cm)
1. *Boundary Seeds*: $D_0(t) = 1$ ($forall t >= 0$) and $D_1(t) = 1$ ($forall t >= 1$).
2. *First Zero Diagonal ($z_1 = 2$)*: Setting $j=0$ in the recurrence:
   $ D_2(t+1) = D_0(t) xor (D_1(t) or D_2(t)) = 1 xor (1 or D_2) = 1 xor 1 = bold(0) quad (forall t >= 1). $
   Hence $D_2(t) = 0$ for all $t >= 2$.
3. *Strict Alternation of Diagonal 3*: Setting $j=1$ and substituting $D_2 = 0$:
   $ D_3(t+1) = D_1(t) xor (D_2(t) or D_3(t)) = 1 xor (0 or D_3) = 1 xor D_3 = bold(not D_3(t)) quad (forall t >= 2). $
   Hence $D_3$ strictly alternates with period 2 ($D_3(t+2) = D_3(t)$).
4. *Unit Absorption of Diagonal 4*: Setting $j=2$ and substituting $D_2 = 0$:
   $ D_4(t+1) = D_2(t) xor (D_3(t) or D_4(t)) = 0 xor (D_3 or D_4) = D_3(t) or D_4(t). $
   Since $D_3(3)=1$, $D_4(4)=1$, and an OR accumulator with 1 is locked: $bold(D_4(t) = 1)$ for all $t >= 4$.
5. *Collapse of Diagonal 5*: Setting $j=3$ for $t >= 4$ and substituting $D_4(t) = 1$:
   $ D_5(t+1) = D_3(t) xor (D_4(t) or D_5(t)) = D_3(t) xor (1 or D_5) = D_3(t) xor 1 = not D_3(t) = bold(D_3(t+1)). $
   Integrating yields $bold(D_5(t) = D_3(t))$ for all $t >= 5$. $bold(qed_1)$
6. *Collapse of Diagonal 6*: Setting $j=4$ for $t >= 4$: $D_6(t+1) = D_4(t) xor (D_5(t) or D_6(t)) = not (D_5(t) or D_6(t))$.
   By induction from $t = 6$ ($D_6(6) = D_5(6) = 0$), $D_5 or D_6 = D_5$, so $D_6(t+1) = not D_5(t) = D_5(t+1)$. Thus $bold(D_6(t) = D_5(t))$ for all $t >= 6$.
7. *The Annihilation of Diagonal 7 ($x xor x = 0$ Cancellation)*: Setting $j=5$:
   $ D_7(t+1) = D_5(t) xor (D_6(t) or D_7(t)) $
   By induction from $t = 7$ ($D_7(7) = 0$): substituting $D_7(t) = 0$ and $D_6(t) = D_5(t)$ yields:
   $ D_7(t+1) = D_5(t) xor (D_5(t) or 0) = D_5(t) xor D_5(t) = bold(0) quad (forall t >= 7). quad bold(qed_2) $

#v(0.2cm)
#line(length: 100%, stroke: 0.5pt + luma(180))
#v(0.08cm)
#align(center)[
  #text(8.5pt, fill: luma(80))[
    *Author*: Umidjon Rasulov (School of Computer and Information Engineering, Inha University in Tashkent). \
    *Official Verification*: Formalized in Lean 4 (`Rule30Submission.original_diagonal_invariants`). \
    Kernel verified via `WolframInstitute/Rule30PrizeKit` (`Check.lean`) with exit code 0, standard axioms only (`propext`, `Quot.sound`).
  ]
]
