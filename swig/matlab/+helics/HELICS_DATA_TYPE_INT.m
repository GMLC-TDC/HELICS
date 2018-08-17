function v = HELICS_DATA_TYPE_INT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783878);
  end
  v = vInitialized;
end
