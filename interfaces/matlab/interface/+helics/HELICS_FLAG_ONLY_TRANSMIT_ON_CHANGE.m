function v = HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 31);
  end
  v = vInitialized;
end
