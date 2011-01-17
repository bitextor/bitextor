#!/usr/bin/env python

#
# 1. Lee contenido en base64
# 2. Extrae el texto encontrado sin etiquetas HTML
# 3. Muestra todo el texto procesado
#
# Formato final del documento:
# texto_plano
#

import sys
import base64
from HTMLParser import HTMLParser

def contar_palabras( data ):
  return len(data.split())

class Parser(HTMLParser):

  def __init__( self ):
    HTMLParser.__init__( self )
    self.contains1 = ("h1", "h2", "h3", "h4")
    self.contains2 = ("p", "li", "a")
    self.script = 0
    self.include1 = 0
    self.include2 = 0

  def handle_starttag( self, tag, attrs ):
    if tag == "script" or tag == "noscript":
      self.script = 1
    if self.script == 0:
      if tag in self.contains1:
        self.include1 = 1
      elif tag in self.contains2:
        self.include2 += 1

  def handle_data( self, data ):
    if self.script == 0:
      if self.include1 == 1:
        if data[-1] == ".":
          print data + "\n"
        else:
          print data + ".\n"
      elif self.include2 > 0:
        palabras = contar_palabras(data)
        if palabras > 4:
          if data[-1] == ".":
            print data + "\n"
          else:
            print data + ".\n"

  def handle_endtag( self, tag ):
    if tag == "script" or tag == "noscript":
      self.script = 0
    if self.script == 0:
      if tag in self.contains1:
        self.include1 = 0
      elif tag in self.contains2:
        self.include2 -= 1

for i in sys.stdin:
  e = base64.b64decode(i)
  if e != "":
    Parser().feed(e)

