// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// xml_out_field.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Notes:
// _______________________________________________________________________________________________

# include <QTextStream>
# include "sheet.h"

namespace xml {
namespace out {

  class
stream_type
{
  public:
    /* ctor */  stream_type( sheet_type const *, QIODevice *) ;


  private:
    static QString indent(int indentLevel);
    static QString escapedText(const QString &str);
    static QString escapedAttribute(const QString &str);

    void generateItem(QTreeWidgetItem *item, int depth);


    QTreeWidget *treeWidget;
    QTextStream out;

    size_t indent_ ;
};


// new line?
// wrapper?
// can it be canceled?
  class
tag_wrapper_type
{
  public:
    /* ctor */  tag_wrapper_type
                 (  char const *   p_tag_name
                  , QTextStream &  out_stream
                  , bool           delay /* = false */
                 )


    bool        is_delayed( )         const { return where_ == e_before; }
    bool        is_started( )         const { return ! is_delayed( ); }
    void        start( )                    ;

    bool        is_in_header( )       const { return where_ == e_in_header; }
    void        finish_header( )            ;


    // overload for a lot of different values
    void        add_attribute
                 (  const char *  p_attribute_name
                  , const char *  

    void        finish( )                   ;

    bool        is_delayed(   )       const { return where_ == e_delayed  ; }
    bool        is_in_header( )       const { return where_ == e_in_header; }
    bool        is_in_body(   )       const { return where_ == e_in_body  ; }
    bool        is_finished(  )       const { return where_ == e_finished ; }

  private:
    char const * const  p_tag_name_ ;
    QTextStream &       out_stream_ ;
    enum
     {  e_delayed           /* tag not yet opened */
      , e_in_header         /* waiting for attributes */
      , e_in_body           /* waiting for body text */
      , e_finished          /* tag is closed */
     }                  where_      ;
};


  bool
XbelGenerator::write(QIODevice *device)
 {
     out.setDevice(device);
     out.setCodec("UTF-8");
     out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE xbel>\n"
         << "<xbel version=\"1.0\">\n";

     for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
         generateItem(treeWidget->topLevelItem(i), 1);

     out << "</xbel>\n";
     return true;
 }



  void
  xml_out_field_type::
indent( int depth)
{
    for ( depth > 0 ) {
        get_out_stream( ) << ' ';
    }
}


// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// xml_out_field.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
