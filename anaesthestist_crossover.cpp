//+------------------------------------------------------------------+
//|                                                 anaesthetist.mq4 |
//|                                           Daniel Ayomikun Obembe |
//|                                       https://www.hizaswenet.com |
//+------------------------------------------------------------------+
#property copyright "Daniel Ayomikun Obembe"
#property link      "https://www.hizaswenet.com"
#property version   "1.00"
#property strict


extern int ma_1_period = 50;        //Period of shorter moving average
extern int ma_2_period = 200;       //Period of longer moving average
bool will_work = true;              //Expert Advisor can function
string symb;                        //Name of currency pair

extern double stoploss = 100;        //stoploss
extern double takeprofit = 100;      //takeprofit

string market_watcher = "Not Modified";


//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick(){
  int order_count,                        //Number of orders
      order_type = -1,
      order_ticket;
  double  ma_1_current_trading,           //Current shorter moving average value, 5min chart
          ma_1_current_alignment,         //Current shorter moving average value, 15min chart
          ma_2_current_trading,           //Current longer moving average value, 5min chart
          ma_2_current_alignment,         //Current longer moving average value, 15min chart
          stoch_trading_current,          //Current value of stochastic oscillator on 5 min chart
          stoch_alignment_current,        //Current value of stochastic oscillator on 15 min chart
          stoch_trading_previous,         //Previous (1 bar) value of stochastic oscillator on 5 min chart
          stoch_alignment_previous,       //Previous (1 brar) value of stochastic oscillator on 15 min chart
          delta_stoch_trading,            //change between previous and current stochastic oscillator on 5min chart
          delta_stoch_alignment,          //change between previous and current stochastic oscillator on 15min chart
          selected_lot_size,              //Amount of lots in selected order
          opened_lot_size,                //Amount of lots in opened order
          minimum_lot_size,               //Mimimum required amount of lots
          step,                           //Step of a lot size change
          free_margin,                    //Current free margin
          lot_price,                      //Price of one lot
          order_price,                    //Price of a selected order
          stop_loss,
          take_profit;
   bool   close_buy = false,              //Criterion for closing Buy order
          close_sell = false,             //Criterion for closing Sell order
          open_buy = false,               //Criterion for opening Buy order
          open_sell = false,              //Criterion for opening Sell order
          ranging_market = false,         //Criterion for closing all trades if market ranging
          trade_answer = false;           //Response from broker regarding trade request


  //  Section 1: Preliminary Processing
  if (Bars < ma_1_period){                //Not enough bars
    Alert("Not enough bars in the window. Anaesthetist cannot function.");
    return;                               //Exit OnTick()
  }
  if (will_work==false){                  //Critical Error
    Alert("Critical error has occured. Anaesthetist cannot function.");
    return;
  }

  // Section 2: Order Accounting (EA only trades one order at a time)
  symb = Symbol();                        //Name of currency pair
  order_count = 0;
                          //Accounting loop
  for (int i=0; i<OrdersTotal(); i++) {   //Note: Tutorial used i<=OrdersTotal()
    if (OrderSelect(i, SELECT_BY_POS)==true){
      if (OrderSymbol()!=symb) continue;  //Skip to next iteration
      if (OrderType()>1) {
        Alert("A pending order has been detected. Anaesthetist cannot function.");
        return;                           // Exit OnTick()
      }
      order_count++;                      //increment order counter
      if (order_count>1){
        Alert("Several market orders. Anaesthetist cannot function.");
        return;
      }
      order_ticket= OrderTicket();
      order_type= OrderType();
      order_price = OrderOpenPrice();
      stop_loss = OrderStopLoss();
      take_profit = OrderTakeProfit();
      selected_lot_size = OrderLots();
    }
/*
    Alert(symb," Order type: ",order_type,
          " Order Price: ", order_price,
          " Stop Loss: ", stop_loss,
          " Take Profit: ", take_profit,
          " Selected Lot Size: ", selected_lot_size);*/
  }
                        //End Of Accounting Loop//


  //Section 3: Specifying Trading Criteria

  //3_a: Tools For Verifying Trend
  ma_1_current_trading = iMA(NULL,5,ma_1_period,0,MODE_EMA,PRICE_TYPICAL,0);
                                    //5 min exponenetial moving average, period 50
  ma_2_current_trading = iMA(NULL,5,ma_2_period,0,MODE_EMA,PRICE_TYPICAL,0);
                                    //5 min exponenetial moving average, period 200
  ma_1_current_alignment = iMA(NULL,15,ma_1_period,0,MODE_EMA,PRICE_TYPICAL,0);
                                    //15 min exponenetial moving average, period 50
  ma_2_current_alignment = iMA(NULL,15,ma_2_period,0,MODE_EMA,PRICE_TYPICAL,0);
                                    //15 min exponenetial moving average, period 200
  bool trading_uptrend = (ma_1_current_trading > ma_2_current_trading);
                                    //true if 5m is in technical uptrend
  bool alignment_uptrend = (ma_1_current_alignment > ma_2_current_alignment);
                                    //true if 15m is in technical uptrend
  bool market_aligned = (trading_uptrend==alignment_uptrend);
                                    //true if both timeframes trend in same
                                    //direction

  //3_b: Tools For Verifying Direction
  stoch_trading_current = iStochastic(NULL,5,5,3,3,MODE_EMA,1,MODE_MAIN,0);
                                    //stochastic oscillator 5 min, current bar
  stoch_trading_previous = iStochastic(NULL,5,5,3,3,MODE_EMA,1,MODE_MAIN,1);
                                    //stochastic oscillator 5 min, previous bar
  stoch_alignment_current = iStochastic(NULL,15,5,3,3,MODE_EMA,1,MODE_MAIN,0);
                                    //stochastic oscillator 15 min, current bar
  stoch_alignment_previous = iStochastic(NULL,15,5,3,3,MODE_EMA,1,MODE_MAIN,1);
                                    //stochastic oscillator 15 min, previous bar
  delta_stoch_trading = stoch_trading_current - stoch_trading_previous;
                                    //change in stoch, 5 min
  delta_stoch_alignment = stoch_alignment_current - stoch_alignment_previous;
                                    //change in stoch, 15 min

  //3_c: Specifying Trading Criteria
  if (market_aligned && market_watcher=="Not Modified") {
    //Alert("Market watcher NOT modified");
    if (trading_uptrend) {      //NOTE_MODIFIED stoch period on close signals.
        //Uptrend
        if((stoch_alignment_current<=10.0 && delta_stoch_alignment>0)
          && (stoch_trading_current<=20.0 && delta_stoch_trading>0)) {
          open_buy = true;
        }
        if(stoch_alignment_current>=80.0 && delta_stoch_alignment<0) { /////!!!!
          close_buy = true;
        }
    }
    if (!trading_uptrend) {
        //Downtrend
        if((stoch_alignment_current>=90.0 && delta_stoch_alignment<0)
          && (stoch_trading_current>=80.0 && delta_stoch_trading<0)) {
          open_sell = true;
        }
        if(stoch_alignment_current<=20.0 && delta_stoch_alignment>0) { ////!!!!
          close_sell = true;
        }
    }
  }

  if (!market_aligned) {
    //Alert("Market watcher BEING modified");
    market_watcher = "Modified";
    if (trading_uptrend) {}
    if (!trading_uptrend) {}
  }


  if (market_aligned && market_watcher=="Modified"){
    //Alert("Market watcher previously modified");
    //Market moves to uptrend when market watch on
    if (alignment_uptrend) {
      if (order_count==1) {
        if(order_type==1) {
          Alert("Downtrend reversed to Uptrend. Closing Sell, opening Buy.");
          close_sell = true;
          market_watcher="Not Modified";
        }
        if(order_type==0) {
          Alert("Uptrend re-established. Leaving Buy open.");
          market_watcher="Not Modified";
        }
      }
      if (order_count==0) {
        //Alert("Buying into uptrend.");
        //open_buy=true;
        market_watcher="Not Modified";
      }
    }
    //Market moves to downtrend when market watch on
    if (!alignment_uptrend) {
      if (order_count==1) {
        if(order_type==0) {
          Alert("Uptrend reversed to Downtrend. Closing Buy, opening Sell.");
          close_buy = true;
          market_watcher="Not Modified";
        }
        if(order_type==1) {
          Alert("Downtrend re-established. Leaving Sell open.");
          market_watcher="Not Modified";
        }
      }
      if (order_count==0) {
        //Alert("Selling with downtrend.");
        //open_sell=true;
        market_watcher="Not Modified";
      }
    }

    //market_watcher="Not Modified";
  }


  //Section 4: Closing Orders
  while(true) {                                   //order closing loop

    if((order_type==0 && close_buy==true)) { //|| ranging_market==true) {        //i.e a Buy order is currently open
      Alert("Attempting to close Buy order ",order_ticket,". Awaiting response.");
      RefreshRates();                             //refresh rates
      trade_answer=OrderClose(order_ticket,selected_lot_size,Bid,2); //attempt to close Buy order
      if (trade_answer==true){
        Alert("Buy order ",order_ticket," successfully closed.");
        break;                                    //exit order closing loop
      }
      if (error_handler(GetLastError())==1) {
        continue;                                 //retry order request
      }
      return;                                     //exit OnInit()
    }

    if(order_type==1 && close_sell==true) { // || ranging_market==true) {       //i.e a Sell order is currently open
      Alert("Attempting to close Sell order ",order_ticket,". Awaiting response.");
      RefreshRates();
      trade_answer=OrderClose(order_ticket,selected_lot_size,Ask,2);
      if (trade_answer==true){
        Alert("Sell order ",order_ticket," successfully closed.");
        break;                                    //exit order closing loop
      }
      if (error_handler(GetLastError())==1) {
        continue;                                 //retry order request
      }
      return;
    }
/*
    if (ranging_market) {
      if (order_type==0) { //&& close_buy==false) {   //existing buy order still open
        double modified_stop = Bid-(newStop(30)*Point*10);
        if (stop_loss<Bid-(newStop(stoploss)*Point*10)) {
          bool res=OrderModify(order_ticket,order_price,NormalizeDouble(modified_stop,Digits),NormalizeDouble(take_profit,Digits),0);
          if(!res) {
            Alert("Error, setting stoploss. Rechecking.");
            if(error_handler(GetLastError())==1) continue;
          }
          else Alert("Stop Loss successfully modified for ranging market.");
          return;
        }
        //else Alert("Stop Loss was already modified.");
        return;
      }

      if (order_type==1) { //&& close_sell==false) { //existing sell order still open
        double modified_stop = Ask+(newStop(30)*Point*10);
        if (stop_loss >= Ask+(newStop(stoploss)*Point*10)) {
          Alert("AAAAAAAAAAAAAAAAHHHHHHHHHHH");
          bool res=OrderModify(order_ticket,order_price,NormalizeDouble(modified_stop,Digits),NormalizeDouble(take_profit,Digits),0);
          if(!res) {
            Alert("Error, setting stoploss. Rechecking.");
            if(error_handler(GetLastError())==1) continue;
          }
          else Alert("Stop Loss successfully modified for ranging market.");
          return;
        }
        //else Alert("Stop Loss was already modified.");
        return;
      }

      return;
    }
*/
    break;                                        //exit while loop
  }


  //Section 5: Computing Order Values
  RefreshRates();
  minimum_lot_size = MarketInfo(symb, MODE_MINLOT);     //Minimal Number Of Lots
  free_margin = AccountFreeMargin();                    //Free Margin
  lot_price = MarketInfo(symb, MODE_MARGINREQUIRED);    //Price of one Lot
  step = MarketInfo(symb, MODE_LOTSTEP);                //Step of a Lot size change

  if(selected_lot_size>0) {
    opened_lot_size = selected_lot_size;
  }
  else
    opened_lot_size = 0.1;             //TO_DO: figure this out later

  if (selected_lot_size<minimum_lot_size) selected_lot_size = minimum_lot_size;

  if (selected_lot_size * lot_price > free_margin){
    Alert("Not enough money in account for ",selected_lot_size," lots.");
    return;                           //exit OnTick
  }


  //Section 6: Opening Orders
  while(true) {
    if(order_count==0 && open_buy==true) {
      RefreshRates();
      stop_loss = Bid - (newStop(stoploss)*Point*10);
      Alert("STOPLOSS :",stop_loss);
      take_profit = Bid + (newStop(takeprofit)*Point*10);
      Alert("TAKEPROFIT :",take_profit);
      Alert("Attempt to open Buy Order. Awaiting response.");
      order_ticket = OrderSend(symb,OP_BUY,opened_lot_size,Ask,2,0,0);
      if(order_ticket > 0) {
        Alert("Opened Buy Order, ",order_ticket);
        OrderSelect(order_ticket, SELECT_BY_TICKET);
        bool res=OrderModify(OrderTicket(),OrderOpenPrice(),NormalizeDouble(stop_loss,Digits),NormalizeDouble(take_profit,Digits),0);
        if(!res) {
          Alert("Error, setting stoploss. Rechecking.");
          if(error_handler(GetLastError())==1) continue;
        }
        else Alert("Stop Loss successfully in place.");
        return;
      }
      if(error_handler(GetLastError())==1) {    //Processing errors
        continue;                               //Retry
      }
      return;                                   //exit OnTick()
    }

    if(order_count==0 && open_sell==true) {
      RefreshRates();
      stop_loss = Ask + (newStop(stoploss)*Point*10);
      Alert("STOPLOSS :",stop_loss);
      take_profit = Ask - (newStop(takeprofit)*Point*10);
      Alert("TAKEPROFIT :",take_profit);
      Alert("Attempt to open Sell Order. Awaiting response.");
      order_ticket = OrderSend(symb,OP_SELL,opened_lot_size,Bid,2,0,0);
      if(order_ticket > 0) {
        Alert("Opened Sell Order, ",order_ticket);
        OrderSelect(order_ticket, SELECT_BY_TICKET);
        bool res=OrderModify(OrderTicket(),OrderOpenPrice(),NormalizeDouble(stop_loss,Digits),NormalizeDouble(take_profit,Digits),0);
        if(!res) {
          Alert("Error, setting stoploss. Rechecking.");
          if(error_handler(GetLastError())==1) continue;
        }
        else Alert("Stop Loss successfully in place.");
        return;
      }
      if(error_handler(GetLastError())==1) {    //Processing errors
        continue;                               //Retry
      }
      return;
    }

    break;
  }
  return;
}


int error_handler(int Error) {
  switch(Error)
       {                                          // Not crucial errors
        case  4: Alert("Trade server is busy. Trying once again..");
           Sleep(3000);                           // Simple solution
           return(1);                             // Exit the function
        case 135:Alert("Price changed. Trying once again..");
           RefreshRates();                        // Refresh rates
           return(1);                             // Exit the function
        case 136:Alert("No prices. Waiting for a new tick..");
           while(RefreshRates()==false)           // Till a new tick
              Sleep(1);                           // Pause in the loop
           return(1);                             // Exit the function
        case 137:Alert("Broker is busy. Trying once again..");
           Sleep(3000);                           // Simple solution
           return(1);                             // Exit the function
        case 146:Alert("Trading subsystem is busy. Trying once again..");
           Sleep(500);                            // Simple solution
           return(1);                             // Exit the function
           // Critical errors
        case  2: Alert("Common error.");
           return(0);                             // Exit the function
        case  5: Alert("Old terminal version.");
           will_work=false;                            // Terminate operation
           return(0);                             // Exit the function
        case 64: Alert("Account blocked.");
           will_work=false;                            // Terminate operation
           return(0);                             // Exit the function
        case 133:Alert("Trading forbidden.");
           return(0);                             // Exit the function
        case 134:Alert("Not enough money to execute operation.");
           return(0);                             // Exit the function
        default: Alert("Some Error occurred: ",Error);  // Other variants
           return(0);                             // Exit the function
       }
}

int newStop(int parameter) {
  int minimum_distance = MarketInfo(symb,MODE_STOPLEVEL);
  if (parameter < minimum_distance) {
    parameter = minimum_distance;
    Alert("Increased distance of stop level");
  }
  return(parameter);
}



//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
  {
  //---

  //---
    return(INIT_SUCCEEDED);
  }


//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
  void OnDeinit(const int reason)
  {
  //---

  }


//+------------------------------------------------------------------+
