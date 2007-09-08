/**
 * kmeditor.cpp
 *
 * Copyright (C)  2007 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "kmcomposereditor.h"
#include "kmcomposewin.h"
#include "kemailquotinghighter.h"
#include "kmcommands.h"
#include "kmmsgdict.h"
#include "kmfolder.h"
#include "maillistdrag.h"

#include <klocale.h>
#include <kmenu.h>

#include <QBuffer>

KMComposerEditor::KMComposerEditor( KMComposeWin *win,QWidget *parent)
 :KMeditor(parent),m_composerWin(win)
{
  QColor defaultColor1( 0x00, 0x80, 0x00 ); // defaults from kmreaderwin.cpp
  QColor defaultColor2( 0x00, 0x70, 0x00 );
  QColor defaultColor3( 0x00, 0x60, 0x00 );
  QColor defaultForeground( qApp->palette().color( QPalette::Text ) );

  //TODO look at background color
  QColor c = Qt::red;
  KConfigGroup readerConfig( KMKernel::config(), "Reader" );
  QColor col1;
  if ( !readerConfig.readEntry(  "defaultColors", true ) )
      col1 = readerConfig.readEntry( "ForegroundColor", defaultForeground );
  else
      col1 = defaultForeground;
  QColor col2 = readerConfig.readEntry( "QuotedText3", defaultColor3  );
  QColor col3 = readerConfig.readEntry( "QuotedText2", defaultColor2  );
  QColor col4 = readerConfig.readEntry( "QuotedText1", defaultColor1  );
  QColor misspelled = readerConfig.readEntry( "MisspelledColor", c  );
  //TODO add it
  //static_cast<Sonnet::KEMailQuotingHighlighter*>(hightighter())->setQuoteColor(col1, col2, col3, col4);
}

KMComposerEditor::~KMComposerEditor()
{
}

QString KMComposerEditor::smartQuote( const QString & msg )
{
  return m_composerWin->smartQuote(msg);
}

QString KMComposerEditor::quotePrefixName() const
{
   return m_composerWin->quotePrefixName();
}

void KMComposerEditor::dropEvent( QDropEvent *e )
{
  const QMimeData *md = e->mimeData();
  if ( KPIM::MailList::canDecode( md ) ) {
    e->accept();
    // Decode the list of serial numbers stored as the drag data
    QByteArray serNums = KPIM::MailList::serialsFromMimeData( md );
    QBuffer serNumBuffer( &serNums );
    serNumBuffer.open( QIODevice::ReadOnly );
    QDataStream serNumStream( &serNumBuffer );
    quint32 serNum;
    KMFolder *folder = 0;
    int idx;
    QList<KMMsgBase*> messageList;
    while ( !serNumStream.atEnd() ) {
      KMMsgBase *msgBase = 0;
      serNumStream >> serNum;
      KMMsgDict::instance()->getLocation( serNum, &folder, &idx );
      if ( folder ) {
        msgBase = folder->getMsgBase( idx );
      }
      if ( msgBase ) {
        messageList.append( msgBase );
      }
    }
    serNumBuffer.close();
    uint identity = folder ? folder->identity() : 0;
    KMCommand *command = new KMForwardAttachedCommand( m_composerWin, messageList,
                                                       identity, m_composerWin );
    command->start();
  } else if ( md->hasFormat( "image/png" ) ) {
    emit attachPNGImageData( e->encodedData( "image/png" ) );
  } else {
    KUrl::List urlList = KUrl::List::fromMimeData( md );
    if ( !urlList.isEmpty() ) {
      e->accept();
      KMenu p;
      const QAction *addAsTextAction = p.addAction( i18n("Add as Text") );
      const QAction *addAsAtmAction = p.addAction( i18n("Add as Attachment") );
      const QAction *selectedAction = p.exec( mapToGlobal( e->pos() ) );
      if ( selectedAction == addAsTextAction ) {
        for ( KUrl::List::Iterator it = urlList.begin();
              it != urlList.end(); ++it ) {
          insert( (*it).url() );
        }
      } else if ( selectedAction == addAsAtmAction ) {
        for ( KUrl::List::Iterator it = urlList.begin();
              it != urlList.end(); ++it ) {
          m_composerWin->addAttach( *it );
        }
      }
    } else if ( md->hasText() ) {
      insert( md->text() );
      e->accept();
    } else {
      kDebug(5006) <<"KMEdit::contentsDropEvent, unable to add dropped object";
      return KMeditor::dropEvent( e );
    }
  }
}

#include "kmcomposereditor.moc"
