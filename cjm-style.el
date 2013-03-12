;; cjm-style.el
;;
;; My coding style for Emacs' CC-Mode:

(defconst cjm-c-style
  '((c-basic-offset            . 2)
    (c-electric-pound-behavior . (alignleft))
    (c-offsets-alist
     . ((access-label         . /)
        (arglist-close        . 0)
        (arglist-intro        . +)
        (case-label           . *)
        (label                . *)
        (member-init-intro    . 0)
        (topmost-intro-cont   . +)
        (statement-case-intro . *)))))
(c-add-style "cjm" cjm-c-style)
