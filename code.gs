function doPost(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Sheet1");
  
  try {
    // Parse the incoming JSON data from the POST request
    var data = JSON.parse(e.postData.contents);

    // Append a new row with the received data
    sheet.appendRow([new Date(), data.PM1_0, data.PM2_5, data.PM10]);

    // Respond to the POST request
    return ContentService.createTextOutput("Data recorded successfully");
  } catch (error) {
    // Log any errors
    Logger.log("Error: " + error.toString());
    return ContentService.createTextOutput("Error processing data: " + error.toString());
  }
}
