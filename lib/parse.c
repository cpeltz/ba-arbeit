#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "definitions.h"
#include "flags.h"
#include "parse.h"
#include "queue.h"

const drive_order_t parse_clean = { {0, 0, 0}, {0, 0, 0, 0}, 0, 0 };

#define PARSE_NEGATIVE      0

int8_t
parse_character(const uint8_t character, drive_order_t * parser, parse_set_t * pset)
{
  int8_t exit_code = 0;

  if (character == 27)
    {
      // ESC beendet alle Eingaben
      pset->status = 0;
      exit_code = -1;
    }
  else
    {
      switch (pset->status)
        {
        case 0:
          // Parser im Initialzustand

          *parser = parse_clean;
          flag_Clear(PARSE_COMMAND_COMPLETE);

          if (character == '>')
            pset->status = 1;
          else
            {
              exit_code = -1;
            }
          break;

        case 1:
          // Letzte Eingabe war '>', beginne mit Parsen
          if (character == '>')
            {
              // Neues Startzeichen ignorieren...
            }
          else if (isprint(character))
            {
              // Nur druckbare Zeichen verarbeiten
              parser->command[parser->command_length++] = character;
              pset->status = 2;
            }
          break;

        case 2:
          // Kommando gelesen, jetzt muss ',' oder Return folgen
          switch (character)
            {
            case ',':
              // Komma ist toll
              pset->status = 3;
              break;

            case '\r':
              // Return beendet Eingabe
              pset->status = 0;
              flag_Set(PARSE_COMMAND_COMPLETE);
              exit_code = 1;
              break;

            default:
              // Ein weiteres Kommando?
              if ((isprint(character)) && (parser->command_length < 3))
                {
                  parser->command[parser->command_length++] = character;
                }
              else
                {
                  pset->status = 0;
                  exit_code = -1;
                }
              break;
            }
          break;

        case 3:
          // Jetzt kommt ein Parameter, also entweder Ziffer oder '-'
          if (isdigit(character))
            {
              pset->number[pset->number_length++] = character;
              pset->status = 4;
            }
          else
            {
              switch (character)
                {
                case '-':
                  // Vorzeichen erkannt
                  flagLocal_Set(&pset->flags, PARSE_NEGATIVE);
                  pset->status = 4;
                  break;
                case '\r':
                  // Return erkannt aber nicht zulässig
                  pset->status = 0;
                  break;
                }
            }
          break;

        case 4:
          // Weitere Ziffern folgen
          if (isdigit(character))
            {
              if (pset->number_length < 5)
                pset->number[pset->number_length++] = character;
            }
          else
            {
              switch (character)
                {
                case ',':
                case '\r':
                  // Parameter abgeschlossen
                  pset->number[pset->number_length] = '\0';
                  if (pset->number_length > 0)
                    {
                      parser->parameter[parser->parameter_count++] = atoi(pset->number);
                      if (flagLocal_Read(&pset->flags, PARSE_NEGATIVE))
                        parser->parameter[parser->parameter_count - 1] *= -1;
                      flagLocal_Clear(&pset->flags, PARSE_NEGATIVE);
                      pset->number_length = 0;
                      switch (character)
                        {
                        case ',':
                          // Wenn noch Platz ist, folgt weiterer Parameter
                          if (parser->parameter_count < 4)
                            pset->status = 3;
                          else
                            pset->status = 0;
                          break;

                        case '\r':
                          // Eingabe abgeschlossen
                          pset->status = 0;
                          flag_Set(PARSE_COMMAND_COMPLETE);
                          exit_code = 1;
                          break;
                        }
                    }
                  else
                    pset->status = 0;
                  break;
                }
            }
          break;
        }
    }
  return exit_code;
}

int8_t
parse_string(const char *progmem_input, drive_order_t * output)
// fertigen String durch den Parser schicken
{
  parse_set_t pset;
  pset.status = 0;
  pset.number_length = 0;
  pset.number[0] = '\0';
  pset.flags = 0;

  parse_character('>', output, &pset);
  char character;
  while ((character = pgm_read_byte(progmem_input++)))
    {
      parse_character(character, output, &pset);
    }

  return parse_character(13, output, &pset);
}

int8_t
parse_string_twi(const char *input, drive_order_t * output)
// fertigen String von TWI durch Parser schicken
// ('>' als erstes Zeichen ist schon dabei)
{
  parse_set_t pset;
  pset.status = 0;
  pset.number_length = 0;
  pset.number[0] = '\0';
  pset.flags = 0;

  char character;
  while ((character = *input++))
    {
      parse_character(character, output, &pset);
    }

  return parse_character(13, output, &pset);
}
