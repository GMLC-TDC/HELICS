function v = HELICS_SEQUENCING_MODE_ORDERED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 125);
  end
  v = vInitialized;
end
