(require 'gnus-cite)
(setq gnus-invalid-group-regexp "[:`'\"]\\|^$")
(setq
 user-full-name "Haojun Bao"
 nnml-directory "~/private/mail/"
 message-default-charset 'utf-8
 gnus-select-method '(nnimap "bhj2"
                             (nnimap-address "bhj2")
                             (nnimap-server-port 143)
                             (nnimap-stream network))
 gnus-signature-limit 500
 message-cite-function 'message-cite-original-without-signature
 )

;; this puts the messages in several groups according to regexp
(add-hook 'gnus-article-display-hook
          '(lambda ()
             (gnus-article-de-quoted-unreadable)
             (gnus-article-emphasize)
             (gnus-article-hide-boring-headers)
             (gnus-article-hide-headers-if-wanted)
             (gnus-article-hide-pgp)
             (gnus-article-highlight)
             (gnus-article-highlight-citation)
             ))


(setq message-send-mail-function 'smtpmail-send-it
      user-mail-address "baohaojun@gmail.com"
      smtpmail-default-smtp-server "bhj2"
      smtpmail-smtp-server "bhj2"
      smtpmail-smtp-service 25
      mail-default-reply-to "haojun.bao@borqs.com"
      smtpmail-local-domain "borqs.com")

(setq gnus-default-charset 'chinese-iso-8bit
      gnus-group-name-charset-group-alist '((".*" . chinese-iso-8bit))
      gnus-summary-show-article-charset-alist
      '((1 . chinese-iso-8bit)
        (2 . gbk)
        (3 . big5)
        (4 . utf-Cool)
        gnus-newsgroup-ignored-charsets
        '(unknown-8bit x-unknown iso-8859-1)))

(setq gnus-group-line-format "%m%M%L%5N/%-5R %25G: %D\n"
      gnus-topic-line-format "%i%n %A (%G) %v\n"
      gnus-summary-line-format ":%U%R%B%s%-80=  %-20,20f|%4L |\n")

(setq gnus-visible-headers
      "^\\(From:\\|To:\\|Cc:\\|Subject:\\|Date:\\|Followup-To:\\|X-Newsreader:\\|User-Agent:\\|X-Mailer:\\)")

(if window-system
    (setq gnus-sum-thread-tree-root ">>"
          gnus-sum-thread-tree-single-indent " >"
          gnus-sum-thread-tree-leaf-with-other "+-> "
          gnus-sum-thread-tree-indent " "
          gnus-sum-thread-tree-vertical "|"
          gnus-sum-thread-tree-single-leaf "`-> "
          gnus-sum-thread-tree-false-root "~>"))

(add-to-list 'gnus-newsgroup-variables 'mm-coding-system-priorities)
(setq gnus-parameters
      (nconc
       ;; Some charsets are just examples!
       '(("\\bcn\\.bbs\\..*" ;; Chinese
          (mm-coding-system-priorities
           '(iso-8859-1 gbk utf-8))))
       gnus-parameters))
;;Local Variables: ***
;;coding: utf-8 ***
;;End: ***
