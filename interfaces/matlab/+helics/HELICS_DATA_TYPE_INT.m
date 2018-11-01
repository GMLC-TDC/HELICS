function v = HELICS_DATA_TYPE_INT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095563);
  end
  v = vInitialized;
end
