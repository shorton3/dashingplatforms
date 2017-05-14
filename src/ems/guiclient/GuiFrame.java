/*******************************************************************************
 *
 * File name:   GuiFrame.java
 * Subsystem:   EMS
 * Description: Implements Gui Client widget frame.
 *
 * Name                 Date       Release
 * -------------------- ---------- ---------------------------------------------
 * Stephen Horton       01/01/2014 Initial release
 *
 *
 ******************************************************************************/
package ems.guiclient;

import ems.idls.session.*;

import java.awt.*;
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.text.*;
import java.util.*;
import javax.swing.*;
import javax.swing.text.*;

import java.lang.*;
import java.util.*;

public class GuiFrame extends JFrame implements ActionListener
{
   private int BufferedLines;
   private int LinesToShow;
   private int CurrentLines;
   private final int WIDTH = 740;
   private final int HEIGHT = 160;

   private JScrollPane scrollPane;
   private JScrollBar vscrollBar;
   private JList list;
   private DefaultListModel model;

   private Session_I agentSession_ = null;

   //constructor
   public GuiFrame()
   {
      super("GUI Client");

      this.BufferedLines = 200; // Lines to buffer in the scrollbar
      this.LinesToShow = LinesToShow;
      this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);

      this.getContentPane().setLayout(new GridLayout(1, 1, 0, 0));
      JPanel logPanel = new JPanel();
      this.getContentPane().add(logPanel);

      GridBagLayout gb = new GridBagLayout();
      GridBagConstraints gbc = new GridBagConstraints();
      logPanel.setLayout(gb);

      //create the list
      list = new JList();
      model = new DefaultListModel();
      list.setModel(model);
      list.setForeground(Color.yellow);
      list.setBackground(Color.black);
      list.setVisibleRowCount(LinesToShow);

      //create the scrollPane to put the list in
      scrollPane = new JScrollPane(list, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
      scrollPane.getHorizontalScrollBar().setUnitIncrement(20);
      scrollPane.setDoubleBuffered(true);
      vscrollBar = scrollPane.getVerticalScrollBar();
      vscrollBar.setUnitIncrement(15);
      gbc.gridheight = 5;
      gbc.anchor = GridBagConstraints.NORTHWEST;
      gbc.fill = GridBagConstraints.BOTH;
      gbc.weighty = 1.0;
      gbc.weightx = 1.0;
      gb.setConstraints(scrollPane, gbc);
      logPanel.add(scrollPane);

      //the Ping Agent Button
      JButton pingButton = new JButton("Ping");
      pingButton.addActionListener(this);
      pingButton.setActionCommand("ping");
      gbc.gridwidth = GridBagConstraints.REMAINDER;
      gbc.gridheight = 1;
      gbc.weightx = 0.0;
      gbc.anchor = GridBagConstraints.NORTHEAST;
      gb.setConstraints(pingButton, gbc);
      logPanel.add(pingButton);

      //the Clear Button
      JButton clearButton = new JButton("Clear");
      clearButton.addActionListener(this);
      clearButton.setActionCommand("clear");
      gb.setConstraints(clearButton, gbc);
      logPanel.add(clearButton);

      setSize(WIDTH, HEIGHT);

      //remain unvisible when instantiated until called later
      setVisible(true);
   }//end constructor

   //actionPerformed method to handle Action Listener Events
   public void actionPerformed(ActionEvent e)
   {
      if (e.getActionCommand().equals("ping"))
      {
         try
         {
            agentSession_.ping();
            addLine("Successfully pinged the Agent"); 
         }//end try
         catch (Exception exception)
         {
            exception.printStackTrace();
         }//end catch
      }//end if
      else if (e.getActionCommand().equals("clear"))
        clearLog();
   }//end actionPerformed

   //method to clear the log
   public void clearLog()
   {
      model.removeAllElements();
   }//end clearLog

   //method to add a line to the log
   public void addLine(String message)
   {
      if (CurrentLines < BufferedLines)
      {
         //add the message
         model.addElement(message);
         CurrentLines++;
         //scroll to the maximum extent
         list.ensureIndexIsVisible(model.getSize() - 1);
      }//end if
      //need to remove a line from the top
      else
      {
         model.removeElementAt(0);
         //add the message
         model.addElement(message);
         //scroll to the maximum extent
         list.ensureIndexIsVisible(model.getSize() - 1);
      }//end else
   }//end addLine

   /** Update/store reference to the remote Agent Session interface object */
   public void setAgentSession(Session_I agentSession)
   {
      agentSession_ = agentSession;
   }//end setAgentSession

}//end class GuiFrame


