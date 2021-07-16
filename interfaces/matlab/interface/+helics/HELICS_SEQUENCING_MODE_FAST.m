function v = HELICS_SEQUENCING_MODE_FAST()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 117);
  end
  v = vInitialized;
end
