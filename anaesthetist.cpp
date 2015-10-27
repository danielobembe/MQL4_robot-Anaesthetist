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
          selected_lot_size,              //Amount of lots in selected order
          opened_lot_size,                //Amount of lots in an opened order
          minimum_lot_size,               //Mimimum required amount of lots
          step,                           //Step of a lot size change
          free_margin,                    //Current free margin
          lot_price,                      //Price of one lot
          order_price,                    //Price of a selected order
          stop_loss,
          take_profit;

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
    Alert(symb," Order type: ",order_type,
          " Order Price: ", order_price,
          " Stop Loss: ", stop_loss,
          " Take Profit: ", take_profit,
          " Selected Lot Size: ", selected_lot_size);
  }
                        //End Of Accounting Loop//


  //Section 3: Specifying Trading Criteria

  //3_a: Verifying Trend
  ma_1_current_trading = iMA(NULL,5,ma_1_period,0,MODE_EMA,PRICE_TYPICAL,0);
  ma_2_current_trading = iMA(NULL,5,ma_2_period,0,MODE_EMA,PRICE_TYPICAL,0);
  ma_1_current_alignment = iMA(NULL,15,ma_1_period,0,MODE_EMA,PRICE_TYPICAL,0);
  ma_2_current_alignment = iMA(NULL,15,ma_2_period,0,MODE_EMA,PRICE_TYPICAL,0);

  bool trading_uptrend = (ma_1_current_trading > ma_2_current_trading);
                                    //true if 5m is in technical uptrend
  bool alignment_uptrend = (ma_1_current_alignment > ma_2_current_alignment);
                                    //true if 15m is in technical uptrend
  bool market_aligned = (trading_uptrend==alignment_uptrend);
                                    //true if both timeframes trend in same
                                    //direction

  if (trading_uptrend && market_aligned) Alert(Symbol()," in an aligned uptrend.");
  if (!trading_uptrend && market_aligned) Alert(Symbol()," in an aligned downtrend.");
  if (!market_aligned) Alert(Symbol()," unaligned. Trading on hold.");

  //if (trading_uptrend) Alert("Currently in uptrend on the 5 min chart.");
  //if (!trading_uptrend) Alert("Currently in downtrend on the 5 min chart.");
  //if (alignment_uptrend) Alert("Currently in uptrend on the 15 min chart.");
  //if (!alignment_uptrend) Alert("Currently in downtrend on the 15 min chart.");
  //if (market_aligned) Alert("Market is aligned. Trading can commence.");
  //if(!market_aligned) Alert("Market is NOT aligned. Trading on hold.");


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
