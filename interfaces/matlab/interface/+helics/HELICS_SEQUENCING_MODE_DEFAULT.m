function v = HELICS_SEQUENCING_MODE_DEFAULT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 129);
  end
  v = vInitialized;
end
