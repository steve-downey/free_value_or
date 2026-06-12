;;; init.el --- batch export init for D4270 slides  -*- lexical-binding: t; -*-

;;; Commentary:
;; Minimal package set for exporting the org deck via org-re-reveal.
;; Derived from the cppnow26/trees machinery, trimmed to what the
;; slide export actually needs.

;;; Code:
(setq custom-file (locate-user-emacs-file "custom.el"))

(require 'package)
(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/") t)
(setq package-user-dir
      (locate-user-emacs-file (concat "elpa-" emacs-version)))

(when (fboundp 'native-comp-available-p)
  (setq package-native-compile (native-comp-available-p)))
(package-initialize)

(let ((package-pinned-packages '((use-package . "melpa")
                                 (bind-key    . "melpa")))
      (has-refreshed nil))
  (defun update-package (p has-refreshed)
    (unless (package-installed-p p)
      (unless has-refreshed
        (message "Refreshing package database...")
        (package-refresh-contents)
        (setq has-refreshed t)
        (message "Done."))
      (package-install p)))
  (dolist (pkg package-pinned-packages)
    (update-package (car pkg) has-refreshed)))

(eval-when-compile
  (require 'use-package))
(require 'use-package-ensure)
(setq use-package-always-ensure t)

;;; Org mode
(use-package org
  :commands (org-mode)
  :mode (("\\.org\\'" . org-mode))
  :custom
  (org-src-fontify-natively t)
  (org-src-preserve-indentation t)
  (org-edit-src-content-indentation 0)
  (org-confirm-babel-evaluate nil)
  (org-export-with-toc t)
  (org-export-headline-levels 8)
  (org-html-htmlize-output-type 'css)
  (org-use-sub-superscripts "{}")
  :config
  (org-babel-do-load-languages
   'org-babel-load-languages
   '((shell . t)
     (emacs-lisp . t)
     (C . t))))

(use-package htmlize)

;; Prefer the tree-sitter C++ mode for fontification when available.
(with-eval-after-load 'org
  (add-to-list 'org-src-lang-modes '("text" . text))
  (when (and (fboundp 'treesit-available-p) (treesit-available-p))
    (when (treesit-language-available-p 'cpp)
      (add-to-list 'org-src-lang-modes '("cpp" . c++-ts)))))

;; Reveal.js + Org mode
(use-package org-re-reveal
  :config
  ;; repo-local clone (see Makefile reveal.js target); needed readable
  ;; on disk for reveal_single_file embedding
  (setq org-re-reveal-root "./reveal.js"))

(use-package ox-gfm
  :after (org))

(use-package org-transclusion
  :after org
  :config
  (org-transclusion-mode 1))

(use-package citeproc :after org)

(provide 'init)
;;; init.el ends here
